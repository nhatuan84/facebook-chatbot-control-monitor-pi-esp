from flask import Flask, request
import requests
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import thread

ACCESS_TOKEN = "EAAE4ZATXWProBAKINIJ8bScZBj0LKYJIOZCQYIGRfZAA8ZChQLU5iPXnngol5ztBDwmJpmgK2GXkx147gBMKXx72GJvjmjeZCppJiJ0AXtNLdS1Lc2aC315tlJqo9Yv4ikoG4bWiFKPM5JlTd926TlIZAZAHZBs7BaeptkKPUP8gk4AZDZD";

history = dict()

set_cmd = 'floor1/room1/led1'
get_cmd = 'floor1/room1/temp1'

def reply(user_id, msg):
    data = {
        "recipient": {"id": user_id},
        "message": {"text": msg}
    }
    resp = requests.post("https://graph.facebook.com/v2.6/me/messages?access_token=" + ACCESS_TOKEN, json=data)
    print(resp.content)


def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))
def on_message(mqttc, obj, msg):
    print(msg.topic+": "+str(msg.payload))
    if msg.topic in history:
        user_id = history[msg.topic]
        reply(user_id, msg.payload)
        history.pop(msg.topic, None)

mqttc = mqtt.Client()
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.connect("localhost", 1883, 60)
mqttc.subscribe("floor1/#", 0)

def mqtt_thread( threadName, delay):
   mqttc.loop_forever()
try:
   thread.start_new_thread( mqtt_thread, ("mqtt-thread", 0, ) )
except:
   print "Error: unable to start thread"

app = Flask(__name__)

@app.route('/', methods=['GET'])
def handle_verification():
    return request.args['hub.challenge']

@app.route('/', methods=['POST'])
def handle_incoming_messages():
    data = request.json
    print(data)
    sender = data['entry'][0]['messaging'][0]['sender']['id']
    message = data['entry'][0]['messaging'][0]['message']['text']
    print(message)
    if(message.startswith('set')):
        arr = message.split()
        l = len(arr)
        if(l == 3 and set_cmd == arr[1]):
            cmd = arr[1]
            val = '1' if arr[2]=='on' else '0'
            publish.single(cmd, val, hostname="localhost")
            history[cmd+'/res'] = sender
            return 'ok'
    elif(message.startswith('get')):
        arr = message.split()
        l = len(arr)
        if(l == 2 and get_cmd == arr[1]):
            cmd = arr[1]
            publish.single(cmd, '', hostname="localhost")
            history[cmd+'/res'] = sender
            return 'ok'
    reply(sender, 'invalid query')
    return "ok"

if __name__ == '__main__':
    app.run(debug=True)

