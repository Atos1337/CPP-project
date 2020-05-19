function disableWithHint(button) {
  button.setAttribute('disabled', true);
}

document.addEventListener('DOMContentLoaded', function () {
  checkButton = document.getElementById('checkZip');
  signButton = document.getElementById('signZip');

  checkButton.setAttribute('disabled', true);
  signButton.setAttribute('disabled', true);

  //disableWithHint(checkButton, "Выберите доверенные сертификаты в настройках");
  //disableWithHint(signButton, "Выберите ключи в настройках");

  checkButton.addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndCheck"}, (response) => {});
          }
        );
  signButton.addEventListener(
        'click', () => {
            chrome.runtime.sendMessage({message: "openAndSign"}, (response) => {});
          }
        );
  document.getElementById('settingsButton').addEventListener('click', function (event) {
    if (chrome.runtime.openOptionsPage) {
      chrome.runtime.openOptionsPage();
    } 
    else {
      window.open(chrome.runtime.getURL('options.html'));
    }
  }, true);
}); 

//