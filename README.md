# Crypto Plugin

Chrome Extension, which allows to sign zip-file or check its signature.

## How to install

Go to extension tab in Chrome, enable Developer mode and click on Load unpacked option, where select app folder, copy ID value.

Now, you need to install host. Go to the corresponding folder, open file ```com.project.native_messaging_host.json``` and write your ID in chrome-extension field.
Выполните ```install_host.sh```

THen you have to build host. You need to install OpenSSL и zlib libraries, other libraries locates in the source code. Run 
```bash
cmake .
make
```
and update Pligin in Chrome (reload button in right-down corner)
## How to use
Open plugin and go to the settings. Select and upload your private key, public key and trusted certificates. Now you can sign zip-file and check its signature using Plugin interface. In addition, Plugin checks signature of every downloaded zip-file. After signing, creates signing copy of original file, which named result.zip.

## How to delete
Run ```uninstall_host.sh```, delete extension from Chrome.

## License
[MIT](https://choosealicense.com/licenses/mit/)
