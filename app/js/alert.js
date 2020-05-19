var id;

chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
    id = request.id;
    Swal.fire({
      icon: request.icon,
      grow: 'fullscreen',
      html: request.text,
      onDestroy: () => (chrome.runtime.sendMessage({message: "Close me pls", id: id}, (response) => {}))
    });
    sendResponse({message: "OK"});
});