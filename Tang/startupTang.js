script.includeFolder("Components")


// create a new component document and pop it up.
var document = db.addDocument(db.types.ComponentDocument);
document.newFile();

document.editor = document.createEditor();
document.editor.show();
