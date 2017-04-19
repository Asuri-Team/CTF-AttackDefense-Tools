# - coding:utf8
from flask import *
import requests

app = Flask(__name__)
app.secret_key = "Hello!"

url = "http://localhost:3000/flag"  # 修改为提交flag的地址
method = "GET"                      # 修改为提交flag的方式
token = "0ade4d3d8b7ed42f"          # 修改为提交flag的token

server_port = 3001


def submit_flag(url, flag, method, token):
    data = {
        "token": token,
        "flag": flag
    }
    r = None
    if str(method).upper() == 'GET':
        r = requests.get(url, params=data)
    elif str(method).upper() == 'POST':
        r = requests.post(url, data=data)
    if r is not None:
        return r.text
    return r


last_flag = {}


@app.route('/flag', methods=['POST'])
def receive_flag():
    flag = request.get_data().strip()
    ip = request.remote_addr
    if not last_flag.has_key(ip):
        last_flag[ip] = set()
    ip_flag_list = last_flag.get(ip, set())
    if flag in ip_flag_list:
        print "Receive flag %s from %s , already submitted." % (flag, ip)
        return ""
    ip_flag_list.add(flag)
    result = ""
    # result = submit_flag(url, flag, method, token) # 不想自动提交 注释掉此行
    print "Receive flag from %s . Submit flag %s result: %s" % (ip, flag, result)
    return ''


if __name__ == '__main__':
    print "post flag to http://local_ip:%s/flag" % (server_port)
    app.run("0.0.0.0", port=server_port)
