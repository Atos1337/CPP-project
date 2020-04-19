// Saves options to chrome.storage
function save_options() {
  var files = document.getElementById('privateKeyFile').files;
  if (files.length) {
    var reader = new FileReader();
    reader.onload = function(){
      var certReader = new FileReader();
      var certfiles = document.getElementById('certificateFile').files;
      certReader.onload = function(){
          chrome.storage.sync.set({
            privateKey: reader.result,
            certificate: certReader.result,
        }, function() {});
      };
      certReader.readAsBinaryString(certfiles[0]);
    };
    reader.readAsBinaryString(files[0]);
  }
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