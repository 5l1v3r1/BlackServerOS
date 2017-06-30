'use scrict'

const jsonfile = require('jsonfile')
const {dialog} = require('electron').remote
const printChat = require('./printChat.js')

// saves graph to savedFile.json
module.exports = function save (cy, path) {
  // parses graph and stores it as an object
  const fullgraph = cy.json()

  dialog.showSaveDialog({
    filters: [{
      name: 'javascript',
      extensions: ['json', 'js']
    }]
  }, (fileToSave) => {
    jsonfile.writeFile(fileToSave, fullgraph, (err) => {
      if (err) {
        throw err
      }
    })
  })

  printChat('graph saved\n👍')
}
