// Saves options to chrome.storage
function save_options() {
  var files = document.getElementById('privateKeyFile').files;
  if (files.length) {
    var reader = new FileReader();
    reader.onload = function(){
      var text = reader.result;
        chrome.storage.sync.set({
        privateKey: text,
      }, function() {});
    };
    reader.readAsBinaryString(files[0]);
  }
  else {
    chrome.storage.sync.set({
      privateKey: "",
    }, function() {});
  }
}

function restore_options() {
  chrome.storage.sync.get({
    privateKey: null
  }, function(items) {  

  var status = document.getElementById('status');
  status.textContent = items.privateKey;
    //alert(items);
    // document.getElementById('privatyKeyFile') = items.privateKey;
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click',
    save_options);