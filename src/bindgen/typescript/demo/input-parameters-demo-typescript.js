import camelCase from "../../camel-case.js"

function inputParametersDemoTypeScript(functionName, indent, parameter, required, modelProperty) {
  let result = ''
  const parameterName = camelCase(parameter.name)
  const inputIdentifier = `${parameterName}Element`
  switch(parameter.type) {
    case 'INPUT_TEXT_FILE':
    case 'INPUT_TEXT_FILE:FILE':
    case 'INPUT_TEXT_STREAM': {
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs input[name=${parameter.name}-file]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('change', (event) => {\n`
      result += `${indent}${indent}const dataTransfer = event.dataTransfer\n`
      result += `${indent}${indent}const files = event.target.files || dataTransfer.files\n\n`
      if (parameter.itemsExpectedMax > 1) {
        const textValue = parameter.type === 'INPUT_TEXT_STREAM' ? 'new TextDecoder().decode(new Uint8Array(arrayBuffer))' : '{ data: new TextDecoder().decode(new Uint8Array(arrayBuffer)), path: files[0].name }'
        result += `${indent}${indent}const inputStrings = await Promise.all(Array.from(files).map(async (file) => { const arrayBuffer = await file.arrayBuffer(); return ${textValue} }))\n`
        result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", inputStrings)\n`
        result += `${indent}${indent}const input = document.querySelector("#${functionName}Inputs sl-input[name=${parameter.name}]")\n`
        const textDataProp = parameter.type.includes('FILE') ? '.path' : '.substring(0,4)'
        result += `${indent}${indent}input.value = model.${modelProperty}.get("${parameterName}").map((x) => x${textDataProp}).toString()\n`
      } else {
        result += `${indent}${indent}const arrayBuffer = await files[0].arrayBuffer()\n`
        const textValue = parameter.type === 'INPUT_TEXT_STREAM' ? 'new TextDecoder().decode(new Uint8Array(arrayBuffer))' : '{ data: new TextDecoder().decode(new Uint8Array(arrayBuffer)), path: files[0].name }'
        result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", ${textValue})\n`
        result += `${indent}${indent}const input = document.querySelector("#${functionName}Inputs sl-input[name=${parameter.name}]")\n`
        result += `${indent}${indent}input.value = model.${modelProperty}.get("${parameterName}").data.substring(0, 50) + ' ...'\n`
      }
      result += `${indent}})\n\n`
    }
      break
    case 'INPUT_BINARY_FILE':
    case 'INPUT_BINARY_FILE:FILE':
    case 'INPUT_BINARY_STREAM': {
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs input[name=${parameter.name}-file]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('change', async (event) => {\n`
      result += `${indent}${indent}const dataTransfer = event.dataTransfer\n`
      result += `${indent}${indent}const files = event.target.files || dataTransfer.files\n\n`
      if (parameter.itemsExpectedMax > 1) {
        const binaryValue = parameter.type === 'INPUT_BINARY_STREAM' ? 'new Uint8Array(arrayBuffer)' : '{ data: new Uint8Array(arrayBuffer), path: file.name }'
        result += `${indent}${indent}const inputBinaries = await Promise.all(Array.from(files).map(async (file) => { const arrayBuffer = await file.arrayBuffer(); return ${binaryValue} }))\n`
        result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", inputBinaries)\n`
        result += `${indent}${indent}const input = document.querySelector("#${functionName}Inputs sl-input[name=${parameter.name}]")\n`
        const binaryDataProp = parameter.type.includes('FILE') ? '.path' : '.subarray(0,4)'
        result += `${indent}${indent}input.value = model.${modelProperty}.get("${parameterName}").map((x) => x${binaryDataProp}).toString()\n`
      } else {
        result += `${indent}${indent}const arrayBuffer = await files[0].arrayBuffer()\n`
        const binaryValue = parameter.type === 'INPUT_BINARY_STREAM' ? 'new Uint8Array(arrayBuffer)' : '{ data: new Uint8Array(arrayBuffer), path: files[0].name }'
        result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", ${binaryValue})\n`
        result += `${indent}${indent}const input = document.querySelector("#${functionName}Inputs sl-input[name=${parameter.name}]")\n`
        const binaryDataProp = parameter.type.includes('FILE') ? '.data' : ''
        result += `${indent}${indent}input.value = model.${modelProperty}.get("${parameterName}")${binaryDataProp}.subarray(0, 50).toString() + ' ...'\n`
      }
      result += `${indent}})\n\n`
    }
      break
    case 'TEXT':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs sl-input[name=${parameter.name}]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('sl-change', (event) => {\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", ${inputIdentifier}.value)\n`
      result += `${indent}})\n\n`
      break
    case 'BOOL':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs sl-checkbox[name=${parameter.name}]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('sl-change', (event) => {\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", ${inputIdentifier}.checked)\n`
      result += `${indent}})\n\n`
      break
    case 'INT':
    case 'UINT':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs sl-input[name=${parameter.name}]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('sl-change', (event) => {\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", parseInt(${inputIdentifier}.value))\n`
      result += `${indent}})\n\n`
      break
    case 'INPUT_JSON':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs input[name=${parameter.name}-file]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('change', async (event) => {\n`
      result += `${indent}${indent}const dataTransfer = event.dataTransfer\n`
      result += `${indent}${indent}const files = event.target.files || dataTransfer.files\n\n`
      result += `${indent}${indent}const arrayBuffer = await files[0].arrayBuffer()\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", JSON.parse(new TextDecoder().decode(new Uint8Array(arrayBuffer))))\n`
      result += `${indent}${indent}const details = document.getElementById("${parameter.name}-input")\n`
      result += `${indent}${indent}details.innerHTML = \`<pre>$\{globalThis.escapeHtml(JSON.stringify(model.${modelProperty}.get("${parameterName}"), globalThis.interfaceTypeJsonReplacer, 2))}</pre>\`\n`
      result += `${indent}${indent}details.disabled = false\n`
      result += `${indent}})\n\n`
      break
    case 'INPUT_IMAGE':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs input[name=${parameter.name}-file]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('change', async (event) => {\n`
      result += `${indent}${indent}const dataTransfer = event.dataTransfer\n`
      result += `${indent}${indent}const files = event.target.files || dataTransfer.files\n\n`
      result += `${indent}${indent}const { image, webWorker } = await readImageFile(null, files[0])\n`
      result += `${indent}${indent}webWorker.terminate()\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", image)\n`
      result += `${indent}${indent}const details = document.getElementById("${parameter.name}-input")\n`
      result += `${indent}${indent}details.innerHTML = \`<pre>$\{globalThis.escapeHtml(JSON.stringify(image, globalThis.interfaceTypeJsonReplacer, 2))}</pre>\`\n`
      result += `${indent}${indent}details.disabled = false\n`
      result += `${indent}})\n\n`
      break
    case 'INPUT_MESH':
      result += `${indent}const ${inputIdentifier} = document.querySelector('#${functionName}Inputs input[name=${parameter.name}-file]')\n`
      result += `${indent}${inputIdentifier}.addEventListener('change', async (event) => {\n`
      result += `${indent}${indent}const dataTransfer = event.dataTransfer\n`
      result += `${indent}${indent}const files = event.target.files || dataTransfer.files\n\n`
      result += `${indent}${indent}const { mesh, webWorker } = await readMeshFile(null, files[0])\n`
      result += `${indent}${indent}webWorker.terminate()\n`
      result += `${indent}${indent}model.${modelProperty}.set("${parameterName}", mesh)\n`
      result += `${indent}${indent}const details = document.getElementById("${parameter.name}-input")\n`
      result += `${indent}${indent}details.innerHTML = \`<pre>$\{globalThis.escapeHtml(JSON.stringify(mesh, globalThis.interfaceTypeJsonReplacer, 2))}</pre>\`\n`
      result += `${indent}${indent}details.disabled = false\n`
      result += `${indent}})\n\n`
      break
    default:
      console.error(`Unexpected interface type: ${parameter.type}`)
      process.exit(1)
  }
  return result
}

export default inputParametersDemoTypeScript