# sudo apt install python3-venv
# python3 -mvenv http-server-env

# pip install flask
# pip install paho-mqtt

# REFERENCES
# https://github.com/eclipse/paho.mqtt.python

# https://github.com/cuttlesoft/flask-echo-server
# export FLASK_RUN_PORT=9000
# export FLASK_RUN_HOST="0.0.0.0"
# export FLASK_ENV=development; flask run

# OR
# python3 app.py

from pprint import pprint
from textwrap import wrap
from flask import Flask, jsonify, request

import paho.mqtt.publish
import traceback
import logging
import json

# Initialize Logging
logging.basicConfig(level=logging.WARNING)  # Global logging configuration
logger = logging.getLogger("Flask app")  # Logger for this module
logger.setLevel(logging.INFO) # Debugging for this file.

app = Flask(__name__)
methods = ["GET", "POST", "PATCH", "DELETE"]

topic = "hat"
auth = {'username':"piZero", 'password':"pihat"}
hostname = "104.248.243.162"
port = 1883
client_id = "fromGoogleForm"

@app.route("/", methods=methods, defaults={"path": ""})
@app.route("/<path:path>", methods=methods)
def hello_world(path):
    divider = "================================================================"
    j = request.get_json()

    print(divider)
    print(f"*** Received data at: {path}")

    # print("\n** data:")
    # print("\n".join(wrap(request.data.decode())))

    # print("\n** form:")
    # pprint(request.form)

    print("\n** json:")
    pprint(j)

    try:
        paho.mqtt.publish.single(
            topic,
            json.dumps(j, separators=(',', ':')),
            qos=2, # for durable connection
            auth=auth,
            hostname=hostname,
            port=port,
            client_id=client_id,
        )
    except Exception as err:
        # logger.error(" paho.mqtt.publish.single Error: " + err.message)
        logging.error(traceback.format_exc())

    print(f"{divider}\n\n")

    return jsonify(
        {
            "endpoint": path,
            "data": request.data.decode("utf-8"),
            "form": request.form,
            "json": request.get_json(),
        }
    )


if __name__ == "__main__":
    # it may not needed as threaded=True by default from version 1.0
    app.run(host="0.0.0.0", port=9000, threaded=True)