"""
zDS (z/OS Data Set) related routes for the Flask application.
Handles data set listing, member listing, and data set/member reading operations.
"""

import hashlib
from flask import Blueprint, jsonify, request
from config import zds

zds_bp = Blueprint('zds', __name__)


@zds_bp.route("/zosmf/restfiles/ds", methods=["GET"])
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


@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>/member", methods=["GET"])
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


@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>", methods=["GET"])
@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>(<member_name>)", methods=["GET"])
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