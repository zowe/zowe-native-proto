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
import json
import warnings
import hashlib
import time
from flask import Flask, jsonify, request
from zowe_apiml_onboarding_enabler_python.registration import PythonEnabler

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import zds_py as zds
import zusf_py as zusf
import zjb_py as zjb

warnings.filterwarnings("ignore", message="Unverified HTTPS request")
ssl._create_default_https_context = ssl._create_unverified_context

base_directory = os.path.dirname(os.path.abspath(__file__))
config_file_path = os.path.join(base_directory, "config/service-configuration.yml")

enabler = PythonEnabler(config_file=config_file_path)
ssl_config = enabler.ssl_config
cert_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("certificate")))
key_file = os.path.abspath(os.path.join(base_directory, ssl_config.get("keystore")))

app = Flask(__name__)


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


@app.route("/zosmf/restfiles/ds", methods=["GET"])
def list_data_sets():
    """
    List the z/OS data sets on a system.

    This endpoint calls the zds.list_datasets function and formats the output similar to the C++ CLI.

    Query Parameters:
        dslevel: Data set name pattern (required) - will have ".**" appended like CLI
        attributes: Show detailed attributes (optional, default: false)
        response-format-csv: Return CSV format (optional, default: false)
        max-entries: Maximum entries to return (optional)
        warn: Show warnings (optional, default: true)
    """
    try:
        dslevel = request.args.get("dslevel")
        attributes = request.args.get("attributes", "false").lower()
        response_format_csv = request.args.get("response-format-csv", "false").lower()
        max_entries = request.args.get("max-entries")
        warn = request.args.get("warn", "true").lower()

        if not dslevel:
            return jsonify({"error": "dslevel parameter is required"}), 400

        if len(dslevel) > 44:
            return (
                jsonify(
                    {"error": "data set pattern exceeds 44 character length limit"}
                ),
                400,
            )

        dsn = dslevel + ".**"

        entries = zds.list_datasets(dsn)

        results = []
        warnings_list = []

        if response_format_csv == "true":
            for entry in entries:
                csv_row = {
                    "name": entry.name if hasattr(entry, "name") else str(entry),
                    "dsorg": entry.dsorg if hasattr(entry, "dsorg") else "",
                    "volser": entry.volser if hasattr(entry, "volser") else "",
                    "migr": entry.migr if hasattr(entry, "migr") else False,
                    "recfm": entry.recfm if hasattr(entry, "recfm") else "",
                }
                results.append(csv_row)
        else:
            for entry in entries:
                if attributes == "true":
                    dataset_info = {
                        "name": entry.name if hasattr(entry, "name") else str(entry),
                        "volser": entry.volser if hasattr(entry, "volser") else "",
                        "dsorg": entry.dsorg if hasattr(entry, "dsorg") else "",
                        "recfm": entry.recfm if hasattr(entry, "recfm") else "",
                        "migr": entry.migr if hasattr(entry, "migr") else False,
                    }
                else:

                    dataset_info = {
                        "name": entry.name if hasattr(entry, "name") else str(entry)
                    }
                results.append(dataset_info)

        if max_entries and max_entries.isdigit():
            max_count = int(max_entries)
            if len(results) > max_count:
                results = results[:max_count]
                if warn == "true":
                    warnings_list.append("results truncated")

        if len(results) == 0 and warn == "true":
            warnings_list.append("no matching results found")

        response = {"items": results, "returnedRows": len(results)}

        if warnings_list and warn == "true":
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not list data set: '{dslevel}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restfiles/ds/<path:data_set_name>/member", methods=["GET"])
def list_data_set_members(data_set_name):
    """
    List the members of a z/OS data set.

    This endpoint calls the zds.list_members function and formats the output similar to the C++ CLI.

    Path Parameters:
        data_set_name: Name of the dataset to list members from (required)

    Query Parameters:
        start: Starting member name for pagination (optional) - currently not supported by C++ function
        pattern: Member name pattern (optional) - currently not supported by C++ function
        max-entries: Maximum entries to return (optional)
        warn: Show warnings (optional, default: true)
    """
    try:
        start = request.args.get("start")
        pattern = request.args.get("pattern")
        max_entries = request.args.get("max-entries")
        warn = request.args.get("warn", "true").lower()

        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400

        members = zds.list_members(data_set_name)

        results = []
        warnings_list = []

        if start:
            warnings_list.append(
                "start parameter provided but not supported by current C++ function"
            )
        if pattern:
            warnings_list.append(
                "pattern parameter provided but not supported by current C++ function"
            )

        for member in members:
            member_info = {
                "name": member.name if hasattr(member, "name") else str(member)
            }
            results.append(member_info)

        if max_entries and max_entries.isdigit():
            max_count = int(max_entries)
            if len(results) > max_count:
                results = results[:max_count]
                if warn == "true":
                    warnings_list.append("results truncated")

        if len(results) == 0 and warn == "true":
            warnings_list.append("no members found")

        response = {
            "items": results,
            "returnedRows": len(results),
            "datasetName": data_set_name,
        }

        if warnings_list and warn == "true":
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not read data set: '{data_set_name}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restfiles/ds/<path:data_set_name>", methods=["GET"])
@app.route("/zosmf/restfiles/ds/<path:data_set_name>(<member_name>)", methods=["GET"])
def read_data_set_or_member(data_set_name, member_name=None):
    """
    Read the contents of a z/OS data set or member.

    This endpoint calls the zds.read_dataset function and formats the output similar to the C++ CLI.

    Path Parameters:
        data_set_name: Name of the dataset to read from (required)
        member_name: Name of the member to read (optional) - if specified, reads member, otherwise reads dataset

    Query Parameters:
        encoding: Encoding/codepage for text conversion (optional)
        return-etag: Return ETag for caching (optional, default: false)
        response-format-bytes: Return as bytes format (optional, default: false)
        volser: Volume serial (currently not implemented in C++ function)
    """
    try:
        encoding = request.args.get("encoding", "")
        return_etag = request.args.get("return-etag", "false").lower()
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()
        volser = request.args.get("volser")

        if member_name:
            full_dsn = f"{data_set_name}({member_name})"
        else:
            full_dsn = data_set_name

        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400

        content = zds.read_dataset(full_dsn, encoding)

        warnings_list = []

        if volser:
            warnings_list.append(
                "volser parameter provided but not supported by current C++ function"
            )

        response = {"records": content, "datasetName": data_set_name}

        if member_name:
            response["memberName"] = member_name

        if return_etag == "true":
            etag = hashlib.md5(content.encode()).hexdigest()
            response["etag"] = etag

        if response_format_bytes == "true":
            response["records"] = [ord(c) for c in content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"

        if warnings_list:
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not read data set: '{full_dsn}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restfiles/fs", methods=["GET"])
def list_uss_files():
    """
    List USS (Unix System Services) files and directories.

    This endpoint calls the zds.list_uss_dir function and formats the output similar to the C++ CLI.

    Query Parameters:
        path: USS file or directory path (required)
        all: Show all files including hidden ones (optional, default: false)
        long: Show long format with detailed attributes (optional, default: false)
    """
    try:
        path = request.args.get("path")
        all_files = request.args.get("all", "false").lower()
        long_format = request.args.get("long", "false").lower()

        if not path:
            return jsonify({"error": "path parameter is required"}), 400

        uss_output = zusf.list_uss_dir(path)

        warnings_list = []

        if all_files == "true":
            warnings_list.append(
                "all parameter provided but not supported by current C++ function"
            )
        if long_format == "true":
            warnings_list.append(
                "long parameter provided but not supported by current C++ function"
            )

        try:
            parsed_output = json.loads(uss_output)
            if isinstance(parsed_output, list):
                items = parsed_output
            else:
                items = [parsed_output]
        except (json.JSONDecodeError, ValueError):

            lines = uss_output.strip().split("\n") if uss_output else []
            items = []
            for line in lines:
                if line.strip():

                    items.append({"name": line.strip(), "type": "unknown"})

        response = {"items": items, "returnedRows": len(items), "path": path}

        if long_format == "true":
            response["format"] = "long"
        else:
            response["format"] = "short"

        if all_files == "true":
            response["showHidden"] = True
        else:
            response["showHidden"] = False

        if warnings_list:
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not list USS files: '{path}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restfiles/fs/<path:file_path>", methods=["GET"])
def read_uss_file(file_path):
    """
    Read the contents of a USS (Unix System Services) file.

    This endpoint calls the zusf.read_uss_file function and formats the output similar to the C++ CLI.

    Path Parameters:
        file_path: USS file path to read (required)

    Query Parameters:
        encoding: Encoding/codepage for text conversion (optional)
        return-etag: Return ETag for caching (optional, default: false)
        response-format-bytes: Return as bytes format (optional, default: false)
    """
    try:
        encoding = request.args.get("encoding", "")
        return_etag = request.args.get("return-etag", "false").lower()
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()

        if not file_path:
            return jsonify({"error": "file path is required"}), 400

        if not file_path.startswith("/"):
            file_path = "/" + file_path

        content = zusf.read_uss_file(file_path, encoding)

        response = {"records": content, "filePath": file_path}

        if return_etag == "true":
            etag_content = f"{file_path}:{len(content)}:{int(time.time())}"
            etag = hashlib.md5(etag_content.encode()).hexdigest()
            response["etag"] = etag

        if response_format_bytes == "true":

            response["records"] = [ord(c) for c in content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not view USS file: '{file_path}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<jobname>/<jobid>", methods=["GET"])
def get_job_status_by_name_and_id(jobname, jobid):
    """
    Get the status of a z/OS job by jobname and jobid.

    This endpoint calls the zjb.get_job_status function and formats the output similar to the C++ CLI.

    Path Parameters:
        jobname: Job name (required)
        jobid: Job ID (required)

    Query Parameters:
        step-data: Include step data (optional, default: N)
    """
    try:
        step_data = request.args.get("step-data", "N").upper()

        if not jobname or not jobid:
            return jsonify({"error": "jobname and jobid are required"}), 400

        job_info = zjb.get_job_status(jobid)

        warnings_list = []

        if step_data == "Y":
            warnings_list.append(
                "step-data parameter provided but not supported by current C++ function"
            )

        response = {
            "jobname": job_info.jobname if hasattr(job_info, "jobname") else jobname,
            "jobid": job_info.jobid if hasattr(job_info, "jobid") else jobid,
            "owner": job_info.owner if hasattr(job_info, "owner") else "",
            "status": job_info.status if hasattr(job_info, "status") else "",
            "retcode": job_info.retcode if hasattr(job_info, "retcode") else "",
            "type": "JOB",
            "url": f"https://{request.host}/zosmf/restjobs/jobs/{jobname}/{jobid}",
        }

        if hasattr(job_info, "full_status") and job_info.full_status:
            response["phase-name"] = job_info.full_status
        if hasattr(job_info, "job_correlator") and job_info.job_correlator:
            response["job-correlator"] = job_info.job_correlator

        if warnings_list:
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not get job status for {jobname}/{jobid} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<correlator>", methods=["GET"])
def get_job_status_by_correlator(correlator):
    """
    Get the status of a z/OS job by job correlator.

    This endpoint calls the zjb.get_job_status function and formats the output similar to the C++ CLI.

    Path Parameters:
        correlator: Job correlator (required)

    Query Parameters:
        step-data: Include step data (optional, default: N)
    """
    try:
        step_data = request.args.get("step-data", "N").upper()

        if not correlator:
            return jsonify({"error": "correlator is required"}), 400

        job_info = zjb.get_job_status(correlator)

        warnings_list = []

        if step_data == "Y":
            warnings_list.append(
                "step-data parameter provided but not supported by current C++ function"
            )

        response = {
            "jobname": job_info.jobname if hasattr(job_info, "jobname") else "",
            "jobid": job_info.jobid if hasattr(job_info, "jobid") else "",
            "owner": job_info.owner if hasattr(job_info, "owner") else "",
            "status": job_info.status if hasattr(job_info, "status") else "",
            "retcode": job_info.retcode if hasattr(job_info, "retcode") else "",
            "job-correlator": correlator,
            "type": "JOB",
            "url": f"https://{request.host}/zosmf/restjobs/jobs/{correlator}",
        }

        if hasattr(job_info, "full_status") and job_info.full_status:
            response["phase-name"] = job_info.full_status

        if warnings_list:
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not get job status for correlator {correlator} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs", methods=["GET"])
def list_jobs():
    """
    List z/OS jobs with optional filtering.

    This endpoint calls the zjb.list_jobs_by_owner function and formats the output similar to the C++ CLI.

    Query Parameters:
        owner: Job owner name (optional, defaults to current user)
        prefix: Job name prefix filter (optional)
        max-entries: Maximum number of jobs to return (optional)
        warn: Show warnings (optional, default: true)
        response-format-csv: Return CSV format (optional, default: false)
    """
    try:
        owner = request.args.get("owner", "*")
        prefix = request.args.get("prefix", "")
        max_entries = request.args.get("max-entries")
        warn = request.args.get("warn", "true").lower()
        response_format_csv = request.args.get("response-format-csv", "false").lower()

        jobs = zjb.list_jobs_by_owner(owner)

        results = []
        warnings_list = []

        if prefix:
            warnings_list.append(
                "prefix parameter provided but not supported by current C++ function"
            )

        for job in jobs:
            if response_format_csv == "true":
                job_info = {
                    "jobid": job.jobid if hasattr(job, "jobid") else "",
                    "retcode": job.retcode if hasattr(job, "retcode") else "",
                    "jobname": job.jobname if hasattr(job, "jobname") else "",
                    "status": job.status if hasattr(job, "status") else "",
                    "job-correlator": (
                        job.job_correlator if hasattr(job, "job_correlator") else ""
                    ),
                }
            else:
                job_info = {
                    "jobname": job.jobname if hasattr(job, "jobname") else "",
                    "jobid": job.jobid if hasattr(job, "jobid") else "",
                    "owner": job.owner if hasattr(job, "owner") else owner,
                    "status": job.status if hasattr(job, "status") else "",
                    "type": "JOB",
                    "url": f"https://{request.host}/zosmf/restjobs/jobs/{job.jobname if hasattr(job, 'jobname') else 'UNKNOWN'}/{job.jobid if hasattr(job, 'jobid') else 'UNKNOWN'}",
                }

                if hasattr(job, "retcode") and job.retcode:
                    job_info["retcode"] = job.retcode
                if hasattr(job, "full_status") and job.full_status:
                    job_info["phase-name"] = job.full_status
                if hasattr(job, "job_correlator") and job.job_correlator:
                    job_info["job-correlator"] = job.job_correlator

            results.append(job_info)

        if max_entries and max_entries.isdigit():
            max_count = int(max_entries)
            if len(results) > max_count:
                results = results[:max_count]
                if warn == "true":
                    warnings_list.append("results truncated")

        if len(results) == 0 and warn == "true":
            warnings_list.append("no jobs found")

        if response_format_csv == "true":
            response = {
                "items": results,
                "returnedRows": len(results),
                "format": "csv",
                "owner": owner,
            }
        else:
            response = {"items": results, "returnedRows": len(results), "owner": owner}

        if warnings_list and warn == "true":
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not list jobs for owner '{owner}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<jobname>/<jobid>/files", methods=["GET"])
def list_job_files_by_name_and_id(jobname, jobid):
    """
    List the spool files for a z/OS job by jobname and jobid.

    This endpoint calls the zjb.list_spool_files function and formats the output similar to the C++ CLI.

    Path Parameters:
        jobname: Job name (required)
        jobid: Job ID (required)

    Query Parameters:
        max-entries: Maximum number of files to return (optional)
        warn: Show warnings (optional, default: true)
        response-format-csv: Return CSV format (optional, default: false)
    """
    try:
        max_entries = request.args.get("max-entries")
        warn = request.args.get("warn", "true").lower()
        response_format_csv = request.args.get("response-format-csv", "false").lower()

        if not jobname or not jobid:
            return jsonify({"error": "jobname and jobid are required"}), 400

        spool_files = zjb.list_spool_files(jobid)

        results = []
        warnings_list = []

        for dd in spool_files:
            if response_format_csv == "true":
                file_info = {
                    "ddn": dd.ddn if hasattr(dd, "ddn") else "",
                    "dsn": dd.dsn if hasattr(dd, "dsn") else "",
                    "key": dd.key if hasattr(dd, "key") else 0,
                    "stepname": dd.stepname if hasattr(dd, "stepname") else "",
                    "procstep": dd.procstep if hasattr(dd, "procstep") else "",
                }
            else:
                file_info = {
                    "ddname": dd.ddn if hasattr(dd, "ddn") else "",
                    "dsname": dd.dsn if hasattr(dd, "dsn") else "",
                    "id": dd.key if hasattr(dd, "key") else 0,
                    "stepname": dd.stepname if hasattr(dd, "stepname") else "",
                    "procstep": dd.procstep if hasattr(dd, "procstep") else "",
                    "jobname": jobname,
                    "jobid": jobid,
                    "uri": f"https://{request.host}/zosmf/restjobs/jobs/{jobname}/{jobid}/files/{dd.key if hasattr(dd, 'key') else 0}/records",
                }

            results.append(file_info)

        if max_entries and max_entries.isdigit():
            max_count = int(max_entries)
            if len(results) > max_count:
                results = results[:max_count]
                if warn == "true":
                    warnings_list.append("results truncated")

        if len(results) == 0 and warn == "true":
            warnings_list.append("no spool files found")

        response = {
            "items": results,
            "returnedRows": len(results),
            "jobname": jobname,
            "jobid": jobid,
        }

        if response_format_csv == "true":
            response["format"] = "csv"
        if warnings_list and warn == "true":
            response["warnings"] = warnings_list
        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not list files for job {jobname}/{jobid} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<correlator>/files", methods=["GET"])
def list_job_files_by_correlator(correlator):
    """
    List the spool files for a z/OS job by job correlator.

    This endpoint calls the zjb.list_spool_files function and formats the output similar to the C++ CLI.

    Path Parameters:
        correlator: Job correlator (required)

    Query Parameters:
        max-entries: Maximum number of files to return (optional)
        warn: Show warnings (optional, default: true)
        response-format-csv: Return CSV format (optional, default: false)
    """
    try:
        max_entries = request.args.get("max-entries")
        warn = request.args.get("warn", "true").lower()
        response_format_csv = request.args.get("response-format-csv", "false").lower()

        if not correlator:
            return jsonify({"error": "correlator is required"}), 400

        spool_files = zjb.list_spool_files(correlator)

        results = []
        warnings_list = []

        for dd in spool_files:
            if response_format_csv == "true":
                file_info = {
                    "ddn": dd.ddn if hasattr(dd, "ddn") else "",
                    "dsn": dd.dsn if hasattr(dd, "dsn") else "",
                    "key": dd.key if hasattr(dd, "key") else 0,
                    "stepname": dd.stepname if hasattr(dd, "stepname") else "",
                    "procstep": dd.procstep if hasattr(dd, "procstep") else "",
                }
            else:
                file_info = {
                    "ddname": dd.ddn if hasattr(dd, "ddn") else "",
                    "dsname": dd.dsn if hasattr(dd, "dsn") else "",
                    "id": dd.key if hasattr(dd, "key") else 0,
                    "stepname": dd.stepname if hasattr(dd, "stepname") else "",
                    "procstep": dd.procstep if hasattr(dd, "procstep") else "",
                    "jobname": dd.jobname if hasattr(dd, "jobname") else "",
                    "jobid": dd.jobid if hasattr(dd, "jobid") else "",
                    "uri": f"https://{request.host}/zosmf/restjobs/jobs/{correlator}/files/{dd.key if hasattr(dd, 'key') else 0}/records",
                }
            results.append(file_info)

        if max_entries and max_entries.isdigit():
            max_count = int(max_entries)
            if len(results) > max_count:
                results = results[:max_count]
                if warn == "true":
                    warnings_list.append("results truncated")

        if len(results) == 0 and warn == "true":
            warnings_list.append("no spool files found")

        response = {
            "items": results,
            "returnedRows": len(results),
            "job-correlator": correlator,
        }

        if response_format_csv == "true":
            response["format"] = "csv"

        if warnings_list and warn == "true":
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not list files for job correlator {correlator} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route(
    "/zosmf/restjobs/jobs/<jobname>/<jobid>/files/<file_id>/records", methods=["GET"]
)
def read_job_file_by_name_and_id(jobname, jobid, file_id):
    """
    Read the contents of a specific spool file for a z/OS job by jobname, jobid and file ID.

    This endpoint calls the zjb.read_spool_file function and formats the output similar to the C++ CLI.

    Path Parameters:
        jobname: Job name (required)
        jobid: Job ID (required)
        file_id: Spool file ID/key (required)

    Query Parameters:
        encoding: Encoding for text conversion (optional)
        response-format-bytes: Return as bytes format (optional, default: false)
    """
    try:
        encoding = request.args.get("encoding", "")
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()

        if not jobname or not jobid or not file_id:
            return jsonify({"error": "jobname, jobid, and file_id are required"}), 400

        try:
            key = int(file_id)
        except ValueError:
            return jsonify({"error": f"file_id must be a number, got: {file_id}"}), 400

        if zjb is None:
            return jsonify({"error": "zjb module not available"}), 500

        content = zjb.read_spool_file(jobid, key)

        response = {
            "records": content,
            "jobname": jobname,
            "jobid": jobid,
            "ddname": f"DD{key:03d}",
            "id": key,
        }

        if encoding:
            response["warnings"] = [
                "encoding parameter provided but handled by C++ function internally"
            ]

        if response_format_bytes == "true":
            response["records"] = [ord(c) for c in content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not view job file for {jobname}/{jobid} with key {file_id} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<correlator>/files/<file_id>/records", methods=["GET"])
def read_job_file_by_correlator(correlator, file_id):
    """
    Read the contents of a specific spool file for a z/OS job by correlator and file ID.

    This endpoint calls the zjb.read_spool_file function and formats the output similar to the C++ CLI.

    Path Parameters:
        correlator: Job correlator (required)
        file_id: Spool file ID/key (required)

    Query Parameters:
        encoding: Encoding for text conversion (optional)
        response-format-bytes: Return as bytes format (optional, default: false)
    """
    try:
        encoding = request.args.get("encoding", "")
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()

        if not correlator or not file_id:
            return jsonify({"error": "correlator and file_id are required"}), 400

        try:
            key = int(file_id)
        except ValueError:
            return jsonify({"error": f"file_id must be a number, got: {file_id}"}), 400

        content = zjb.read_spool_file(correlator, key)

        response = {
            "records": content,
            "job-correlator": correlator,
            "ddname": f"DD{key:03d}",
            "id": key,
        }

        if encoding:
            response["warnings"] = [
                "encoding parameter provided but handled by C++ function internally"
            ]

        if response_format_bytes == "true":
            response["records"] = [ord(c) for c in content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"
        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not view job file for correlator {correlator} with key {file_id} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<jobname>/<jobid>/files/JCL/records", methods=["GET"])
def read_job_jcl_by_name_and_id(jobname, jobid):
    """
    Read the JCL records for a z/OS job by jobname and jobid.

    This endpoint calls the zjb.get_job_jcl function and formats the output similar to the C++ CLI.

    Path Parameters:
        jobname: Job name (required)
        jobid: Job ID (required)

    Query Parameters:
        encoding: Encoding for text conversion (optional)
        response-format-bytes: Return as bytes format (optional, default: false)
    """
    try:
        encoding = request.args.get("encoding", "")
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()

        if not jobname or not jobid:
            return jsonify({"error": "jobname and jobid are required"}), 400

        jcl_content = zjb.get_job_jcl(jobid)

        response = {
            "records": jcl_content,
            "jobname": jobname,
            "jobid": jobid,
            "ddname": "JCL",
            "type": "JCL",
        }

        if encoding:
            response["warnings"] = [
                "encoding parameter provided but handled by C++ function internally"
            ]

        if response_format_bytes == "true":
            response["records"] = [ord(c) for c in jcl_content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not view JCL for job {jobname}/{jobid} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.route("/zosmf/restjobs/jobs/<correlator>/files/JCL/records", methods=["GET"])
def read_job_jcl_by_correlator(correlator):
    """
    Read the JCL records for a z/OS job by job correlator.

    This endpoint calls the zjb.get_job_jcl function and formats the output similar to the C++ CLI.

    Path Parameters:
        correlator: Job correlator (required)

    Query Parameters:
        encoding: Encoding for text conversion (optional)
        response-format-bytes: Return as bytes format (optional, default: false)
    """
    try:
        encoding = request.args.get("encoding", "")
        response_format_bytes = request.args.get(
            "response-format-bytes", "false"
        ).lower()

        if not correlator:
            return jsonify({"error": "correlator is required"}), 400

        jcl_content = zjb.get_job_jcl(correlator)
        response = {
            "records": jcl_content,
            "job-correlator": correlator,
            "ddname": "JCL",
            "type": "JCL",
        }

        if encoding:
            response["warnings"] = [
                "encoding parameter provided but handled by C++ function internally"
            ]

        if response_format_bytes == "true":
            response["records"] = [ord(c) for c in jcl_content]
            response["format"] = "bytes"
        else:
            response["format"] = "text"

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not view JCL for job correlator {correlator} - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found"}), 404


@app.errorhandler(500)
def internal_error(error):
    return jsonify({"error": "Internal server error"}), 500


if __name__ == "__main__":
    enabler.register()
    app.run(host="0.0.0.0", port=10018, ssl_context=(cert_file, key_file), debug=False)
