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
  var privateKeyFiles = document.getElementById('privateKeyFile').files;
  var certificateFiles = document.getElementById('certificateFile').files;
  var certificatesStore = document.getElementById('certificatesStore').files;
  if (privateKeyFiles.length) {
      readFile(privateKeyFiles[0], (privateKey) => {
      chrome.storage.sync.set({privateKey: privateKey}, function() {});
      restore_options();
    });
  }
  if (certificateFiles.length) {
    readFile(certificateFiles[0], (certificate) => {
      chrome.storage.sync.set({certificate: certificate}, function() {});
      restore_options();
    });
  }
  if (certificatesStore.length) {
    readFiles(certificatesStore, (certificates) => {
      chrome.storage.sync.set({certificatesStore: certificates}, function() {});
      restore_options();
    });
  }
}

function restore_options() {
  chrome.storage.sync.get({
    privateKey: null,
    certificate: null,
    certificatesStore: null
  }, function(items) {  

  if (items.privateKey) {
    var privateKeyStatus = document.getElementById('privateKeyStatus');
    privateKeyStatus.textContent = "Приватный ключ загружен";
    //privateKeyStatus.textContent = items.privateKey;
  }
  if (items.certificate) {
    var certificateStatus = document.getElementById('certificateStatus');
    certificateStatus.textContent = "Публичный ключ загружен";
    //certificateStatus.textContent = items.certificate;
  }

  if (items.certificatesStore && items.certificatesStore.length >= 1) {
    var certificatesStoreStatus = document.getElementById('certificatesStoreStatus');
    certificatesStoreStatus.textContent = "Загружены " + items.certificatesStore.length + " доверенных сертификата"
  }
  });
}
document.addEventListener('DOMContentLoaded', restore_options);
document.getElementById('save').addEventListener('click',
    save_options);