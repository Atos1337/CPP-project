Установка
---------
Сначала необходимо установить плагин:

Chrome->Extensions->Developer mode->Load unpacked указать папку app

Скопировать ID плагина и выставить его в host/com.project.native_messaging_host.json в поле chrome-extension
(ID может меняться при загрузке неупакованных плагинов)

Теперь:
```
cd host/
bash install_host.sh
make
```

Удаление
--------

Удалить плагин из хрома

Удалить хост:
```
bash host/uninstall_host.sh
```
