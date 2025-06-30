"""
zJB (z/OS Job) related routes for the Flask application.
Handles job status checking, listing, spool file operations, and JCL reading.
"""

from flask import Blueprint, jsonify, request
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from bindings import zjb_py as zjb

zjb_bp = Blueprint('zjb', __name__)


@zjb_bp.route("/zosmf/restjobs/jobs/<jobname>/<jobid>", methods=["GET"])
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


@zjb_bp.route("/zosmf/restjobs/jobs/<correlator>", methods=["GET"])
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


@zjb_bp.route("/zosmf/restjobs/jobs", methods=["GET"])
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


@zjb_bp.route("/zosmf/restjobs/jobs/<jobname>/<jobid>/files", methods=["GET"])
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
    
@zjb_bp.route("/zosmf/restjobs/jobs/<correlator>/files", methods=["GET"])
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


@zjb_bp.route(
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


@zjb_bp.route("/zosmf/restjobs/jobs/<correlator>/files/<file_id>/records", methods=["GET"])
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


@zjb_bp.route("/zosmf/restjobs/jobs/<jobname>/<jobid>/files/JCL/records", methods=["GET"])
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


@zjb_bp.route("/zosmf/restjobs/jobs/<correlator>/files/JCL/records", methods=["GET"])
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