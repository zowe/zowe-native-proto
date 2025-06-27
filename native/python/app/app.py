"""
* This program and the accompanying materials are made available under the terms of the
* Eclipse Public License v2.0 which accompanies this distribution, and is available at
* https://www.eclipse.org/legal/epl-v20.html
*
* SPDX-License-Identifier: EPL-2.0
*
* Copyright Contributors to the Zowe Project.
"""

import os
import sys
import ssl
import yaml
import warnings
from flask import Flask, jsonify
from zowe_apiml_onboarding_enabler_python.registration import PythonEnabler

# Import route blueprints
from routes import zds_bp, zusf_bp, zjb_bp

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

warnings.filterwarnings("ignore", message="Unverified HTTPS request")
ssl._create_default_https_context = ssl._create_unverified_context

base_directory = os.path.dirname(os.path.abspath(__file__))
config_file_path = os.path.join(base_directory, "config/service-configuration.yml")

enabler = PythonEnabler(config_file=config_file_path)
ssl_config = enabler.ssl_config
cert_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("certificate")))
key_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("keystore")))

app = Flask(__name__)

# Register blueprints
app.register_blueprint(zds_bp)
app.register_blueprint(zusf_bp)
app.register_blueprint(zjb_bp)


@app.route("/pythonservice/registerInfo", methods=["GET"])
def register_python_enabler():
    """Test Endpoint to manually register the service."""
    if enabler:
        try:
            enabler.register()
            return jsonify(
                {"message": "Registered with Python eureka client to Discovery service"}
            )
        except Exception as e:
            return jsonify({"error": f"Failed to register: {str(e)}"}), 500
    else:
        return jsonify({"error": "Enabler not initialized"}), 500


@app.route("/pythonservice/unregisterInfo", methods=["GET"])
def unregister_python_enabler():
    """Test Endpoint to manually unregister the service."""
    if enabler:
        try:
            enabler.unregister()
            return jsonify(
                {"message": "Unregistered Python eureka client from Discovery service"}
            )
        except Exception as e:
            return jsonify({"error": f"Failed to unregister: {str(e)}"}), 500
    else:
        return jsonify({"error": "Enabler not initialized"}), 500


@app.route("/pythonservice/apidoc", methods=["GET"])
def get_swagger():
    try:
        with open(os.path.join(base_directory, "pythonSwagger.json")) as f:
            data = yaml.safe_load(f)
        return jsonify(data)
    except FileNotFoundError:
        return jsonify({"error": "Swagger documentation not found"}), 404


@app.route("/pythonservice/application/info", methods=["GET"])
def get_application_info():
    return jsonify(
        {
            "build": {
                "name": "python-service",
                "operatingSystem": "z/OS",
                "time": 1660222556.497,
                "machine": "mainframe-system",
                "number": "n/a",
                "version": "2.3.0",
            }
        }
    )


@app.route("/pythonservice/application/health", methods=["GET"])
def get_application_health():
    return jsonify({"status": "UP"})


@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found"}), 404


@app.errorhandler(500)
def internal_error(error):
    return jsonify({"error": "Internal server error"}), 500


if __name__ == "__main__":
    enabler.register()
    app.run(host="0.0.0.0", port=10018, ssl_context=(cert_file, key_file), debug=False)
