import 'regenerator-runtime/runtime'
import React from 'react'
import { MAIN } from '../wasm/index'
import wasmUrl from '../wasm/build/main/bin/index.wasm'
import { createRoot } from 'react-dom/client'
import { SimulationHandlerComponent } from './simulation/SimulationHandlerComponent'

MAIN({
    wasmFile: wasmUrl,
}).then(async (main) => {
    const controller = new main.SimulationController();
    const root = createRoot(document.getElementById('drift')!)
    root.render(<SimulationHandlerComponent controller={controller} />)
})