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
from fastapi import FastAPI, HTTPException, Query
from fastapi.responses import JSONResponse
from typing import Optional
import sys
import ssl
import yaml
import uvicorn
from zowe_apiml_onboarding_enabler_python.registration import PythonEnabler

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import zds_py as zds

base_directory = os.path.dirname(os.path.abspath(__file__))
config_file_path = os.path.join(base_directory, 'config/service-configuration.yml')

enabler = PythonEnabler(config_file=config_file_path)

ssl_config = enabler.ssl_config
cert_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("certificate")))
key_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("keystore")))

if not cert_file or not key_file:
    raise ValueError("SSL certificate or key file is missing in service-configuration.yml")

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain(certfile=cert_file, keyfile=key_file)

app = FastAPI(title="z/OS REST Files Service", description="FastAPI implementation of z/OS REST Files Service")

@app.get("/zosservice/registerInfo")
def register_python_enabler():
    """Test Endpoint to manually register the service."""
    enabler.register()
    return {"message": "Registered with Python eureka client to Discovery service"}

@app.get("/zosservice/unregisterInfo")
def unregister_python_enabler():
    """Test Endpoint to manually unregister the service."""
    enabler.unregister()
    return {"message": "Unregistered Python eureka client from Discovery service"}

@app.get("/zosservice/apidoc")
def get_swagger():
    with open('zosSwagger.json') as f:
        data = yaml.safe_load(f)
    return JSONResponse(content=data)

@app.get("/zosservice/application/info")
def get_application_info():
    return {
        "build": {
            "name": "zos-restfiles-service",
            "operatingSystem": "z/OS",
            "time": 1660222556.497,
            "machine": "mainframe-system",
            "number": "n/a",
            "version": "1.0.0",
        }
    }

@app.get("/zosservice/application/health")
def get_application_health():
    return {"status": "UP"}

@app.get("/zosmf/restfiles/ds")
def list_data_sets(
    dslevel: str = Query(..., description="Data set name pattern (e.g., USER.*.DATASET)"),
    volser: Optional[str] = Query(None, description="Volume serial number"),
    start: Optional[str] = Query(None, description="Starting dataset name for pagination")
):
    """
    List the z/OS data sets on a system.
    
    This endpoint calls the zds.list_data_sets function with the provided parameters.
    """
    try:
        # Prepare parameters for the C++ function call
        params = {
            'dslevel': dslevel
        }
        
        # Add optional parameters if provided
        if volser is not None:
            params['volser'] = volser
        if start is not None:
            params['start'] = start
        
        # Call the SWIG-generated function
        result = zds.list_data_sets(**params)
        
        # Return the result as JSON
        return JSONResponse(content=result)
        
    except Exception as e:
        # Handle errors from the C++ function
        raise HTTPException(
            status_code=500, 
            detail=f"Error calling zds.list_data_sets: {str(e)}"
        )


if __name__ == "__main__":
    # Load SSL configuration
    enabler.register()
    uvicorn.run(app, host="0.0.0.0", port=10018, ssl_certfile=cert_file,
                ssl_keyfile=key_file)