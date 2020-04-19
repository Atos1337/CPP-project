document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('checkCerty').addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndCheck"}, (response) => {});
          }
        );
}); 

//