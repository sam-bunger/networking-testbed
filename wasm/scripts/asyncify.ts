import ts from 'typescript'
import fs from 'fs'

const fileName = './build/drift/drift.d.ts'
const compileOptions = {
  target: ts.ScriptTarget.ES5,
  module: ts.ModuleKind.CommonJS,
}

;(async () => {
  const program = ts.createProgram({
    rootNames: [fileName],
    options: compileOptions,
  })

  const transformer: ts.TransformerFactory<ts.SourceFile> = (context) => {
    return (node: ts.SourceFile) => {
      const visit = (n: ts.Node): ts.Node => {
        if (ts.isMethodDeclaration(n)) {
          //@ts-ignore
          if (n.type && n.type.typeName && n.type.typeName.escapedText === 'Promise') return n
          return context.factory.createMethodDeclaration(
            n.decorators,
            n.modifiers,
            n.asteriskToken,
            n.name,
            n.questionToken,
            n.typeParameters,
            n.parameters,
            context.factory.createTypeReferenceNode('Promise', [n.type!]),
            n.body
          )
        } else if (ts.isFunctionDeclaration(n)) {
          // If function already returns a promise, don't wrap it
          //@ts-ignore
          if (n.type && n.type.typeName && n.type.typeName.escapedText === 'Promise') return n
          return context.factory.createFunctionDeclaration(
            n.decorators,
            n.modifiers,
            n.asteriskToken,
            n.name,
            n.typeParameters,
            n.parameters,
            context.factory.createTypeReferenceNode('Promise', [n.type!]),
            n.body
          )
        }
        return ts.visitEachChild(n, visit, context)
      }
      return ts.visitNode(node, visit)
    }
  }

  const r = ts.transform<ts.SourceFile>(
    program.getSourceFile(fileName)!,
    [transformer],
    compileOptions
  )

  const printer = ts.createPrinter()
  const result = printer.printNode(
    ts.EmitHint.Unspecified,
    r.transformed[0],
    program.getSourceFile(fileName)!
  )

  fs.writeFileSync(fileName, result)
})()
