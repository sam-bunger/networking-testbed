Module['CONTACT_ADDED_CALLBACK_SIGNATURE'] = 'iiiiiiii'
Module['CONTACT_DESTROYED_CALLBACK_SIGNATURE'] = 'ii'
Module['CONTACT_PROCESSED_CALLBACK_SIGNATURE'] = 'iiii'
Module['INTERNAL_TICK_CALLBACK_SIGNATURE'] = 'vif'

// Reassign global Ammo to the loaded module:
this['Main'] = Module

/**
 * We need to overwrite this function so that we can create our own web workers via webpack, and pass them in here.
 * @param {*} url - Now the url is ignored
 */
function _emscripten_create_worker(url) {
  url = UTF8ToString(url)
  var id = Browser.workers.length
  var info = {
    worker: Module[`worker_${id}`],
    callbacks: [],
    awaited: 0,
    buffer: 0,
    bufferSize: 0,
  }
  info.worker.onmessage = function info_worker_onmessage(msg) {
    if (msg.data.dxe !== 'dxe') return // not a dxe message
    if (ABORT) return
    var info = Browser.workers[id]
    if (!info) return // worker was destroyed meanwhile
    var callbackId = msg.data['callbackId']
    var callbackInfo = info.callbacks[callbackId]
    if (!callbackInfo) return // no callback or callback removed meanwhile
    // Don't trash our callback state if we expect additional calls.
    if (msg.data['finalResponse']) {
      info.awaited--
      info.callbacks[callbackId] = null // TODO: reuse callbackIds, compress this
    }
    var data = msg.data['data']
    if (data) {
      if (!data.byteLength) data = new Uint8Array(data)
      if (!info.buffer || info.bufferSize < data.length) {
        if (info.buffer) _free(info.buffer)
        info.bufferSize = data.length
        info.buffer = _malloc(data.length)
      }
      HEAPU8.set(data, info.buffer)
      callbackInfo.func(info.buffer, data.length, callbackInfo.arg)
    } else {
      callbackInfo.func(0, 0, callbackInfo.arg)
    }
  }
  Browser.workers.push(info)
  return id
}

function _emscripten_call_worker(id, funcName, data, size, callback, arg) {
  funcName = UTF8ToString(funcName)
  var info = Browser.workers[id]
  var callbackId = -1
  if (callback) {
    // If we are waiting for a response from the worker we need to keep
    // the runtime alive at least long enough to receive it.
    // The corresponding runtimeKeepalivePop is in the `finalResponse`
    // handler above.

    callbackId = info.callbacks.length
    info.callbacks.push({
      func: getWasmTableEntry(callback),
      arg: arg,
    })
    info.awaited++
  }
  var transferObject = {
    dxe: 'dxe',
    funcName: funcName,
    callbackId: callbackId,
    data: data ? new Uint8Array(HEAPU8.subarray(data, data + size)) : 0,
  }
  if (data) {
    info.worker.postMessage(transferObject, [transferObject.data.buffer])
  } else {
    info.worker.postMessage(transferObject)
  }
}

function _emscripten_worker_respond(data, size) {
  if (workerResponded) throw 'already responded with final response!'
  workerResponded = true
  var transferObject = {
    dxe: 'dxe',
    callbackId: workerCallbackId,
    finalResponse: true,
    data: data ? new Uint8Array(HEAPU8.subarray(data, data + size)) : 0,
  }
  if (data) {
    postMessage(transferObject, [transferObject.data.buffer])
  } else {
    postMessage(transferObject)
  }
}

;(function () {
  var messageBuffer = null,
    buffer = 0,
    bufferSize = 0

  function flushMessages() {
    if (!messageBuffer) return
    if (runtimeInitialized) {
      var temp = messageBuffer
      messageBuffer = null
      temp.forEach(function (message) {
        onmessage(message)
      })
    }
  }

  function messageResender() {
    flushMessages()
    if (messageBuffer) {
      setTimeout(messageResender, 100) // still more to do
    }
  }

  onmessage = (msg) => {
    if (msg.data.dxe !== 'dxe') return // not a dxe message
    // if main has not yet been called (mem init file, other async things), buffer messages
    if (!runtimeInitialized) {
      if (!messageBuffer) {
        messageBuffer = []
        setTimeout(messageResender, 100)
      }
      messageBuffer.push(msg)
      return
    }
    flushMessages()

    var func = Module['_' + msg.data['funcName']]
    if (!func) throw 'invalid worker function to call: ' + msg.data['funcName']
    var data = msg.data['data']
    if (data) {
      if (!data.byteLength) data = new Uint8Array(data)
      if (!buffer || bufferSize < data.length) {
        if (buffer) _free(buffer)
        bufferSize = data.length
        buffer = _malloc(data.length)
      }
      HEAPU8.set(data, buffer)
    }

    workerResponded = false
    workerCallbackId = msg.data['callbackId']
    if (data) {
      func(buffer, data.length)
    } else {
      func(0, 0)
    }
  }
})()