// Saves options to chrome.storage

function readFile(file, callback) {
  var reader = new FileReader();
  reader.onload = function() { callback(reader.result); };
  reader.readAsBinaryString(file);
}

function readFiles(files, callback) {
  var result = new Array();
  function readFileByIndex(index) {
    if (index >= files.length) {
      callback(result);
      return;
    }
    readFile(files[index], (res) => {
      result.push(res);
      readFileByIndex(index + 1);
    })
  }
  readFileByIndex(0);
}

function save_options() {

  var privateKeyFile = document.getElementById('privateKeyFile').files[0];
  var certificateFile = document.getElementById('certificateFile').files[0];
  var certificatesStore = document.getElementById('certificatesStore').files;
  readFile(privateKeyFile, (privateKey) => {
    chrome.storage.sync.set({privateKey: privateKey}, function() {});
  });
  readFile(certificateFile, (certificate) => {
    chrome.storage.sync.set({certificate: certificate}, function() {});
  });
  readFiles(certificatesStore, (certificates) => {
    chrome.storage.sync.set({certificatesStore: certificates}, function() {});
  });
}

function restore_options() {
  chrome.storage.sync.get({
    privateKey: null,
    certificate: null,
  }, function(items) {  

  var status = document.getElementById('status');
  status.textContent = items.privateKey;

  var status2 = document.getElementById('status2');
  status2.textContent = items.certificate;
    //alert(items);
    // document.getElementById('privatyKeyFile') = items.privateKey;
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click',
    save_options);