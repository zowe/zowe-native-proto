"""
zDS (z/OS Data Set) related routes for the Flask application.
Handles data set listing, member listing, and data set/member reading operations.
"""

import hashlib
from flask import Blueprint, jsonify, request
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from bindings import zds_py as zds

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
    
@zds_bp.route("/zosmf/restfiles/ds/<path:full_path>", methods=["PUT"])
def write_data_set_or_member(full_path):
    """
    Write data to a z/OS data set or member.

    Path Parameters:
        full_path: Full path including optional volser and data set name with optional member
                  Format: [-(*<volser>*)/]*<data-set-name>*[(*<member-name>*)]

    Query Parameters:
        encoding: Encoding/codepage for text conversion (optional)
        etag: ETag for optimistic locking (optional)
        etag-only: Return only ETag in response (optional, default: false)
        pipe-path: Path for streamed writing (currently not implemented)
    """
    try:
        encoding = request.args.get("encoding", "")
        etag = request.args.get("etag", "")
        etag_only = request.args.get("etag-only", "false").lower()
        pipe_path = request.args.get("pipe-path")

        volser = None
        data_set_name = None
        member_name = None
        
        if full_path.startswith("-(*") and "*)/" in full_path:
            volser_end = full_path.find("*)/")
            volser = full_path[3:volser_end]
            remaining_path = full_path[volser_end + 3:]
        else:
            remaining_path = full_path
        
        if "(*" in remaining_path and remaining_path.endswith("*)"):
            member_start = remaining_path.rfind("(*")
            data_set_name = remaining_path[:member_start]
            member_name = remaining_path[member_start + 2:-2]
        else:
            data_set_name = remaining_path

        if member_name:
            full_dsn = f"{data_set_name}({member_name})"
        else:
            full_dsn = data_set_name

        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400

        if request.is_json:
            json_data = request.get_json()
            if "records" in json_data:
                if isinstance(json_data["records"], list):
                    data = ''.join(chr(b) for b in json_data["records"])
                else:
                    data = str(json_data["records"])
            else:
                data = str(json_data)
        else:
            data = request.get_data(as_text=True)

        if not data:
            return jsonify({"error": "request body cannot be empty"}), 400

        warnings_list = []
        if pipe_path:
            warnings_list.append(
                "pipe-path parameter provided but not supported by current function"
            )
        if volser:
            warnings_list.append(
                "volser parameter provided but not supported by current C++ function"
            )

        new_etag = zds.write_dataset(full_dsn, data, encoding, etag)

        if etag_only == "true":
            response = {"etag": new_etag}
        else:
            response = {
                "message": f"Wrote data to '{full_dsn}'",
                "datasetName": data_set_name,
                "etag": new_etag
            }

            if member_name:
                response["memberName"] = member_name

        if warnings_list:
            response["warnings"] = warnings_list

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not write to data set: '{full_dsn}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )


@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>/<member_name>", methods=["POST"])
def create_member(data_set_name, member_name):
    """
    Create a new member in a z/OS data set.
    
    Path Parameters:
        data_set_name: Name of the dataset (required)
        member_name: Name of the member to create (required)
    """
    try:
        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400
        
        if not member_name:
            return jsonify({"error": "member name is required"}), 400

        full_dsn = f"{data_set_name}({member_name})"
        
        zds.create_member(full_dsn)
        
        response = {
            "message": f"Data set and/or member created: '{full_dsn}'",
            "datasetName": data_set_name,
            "memberName": member_name
        }
        
        return jsonify(response), 201

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not create member: '{full_dsn}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    

@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>", methods=["POST"])
def create_dataset(data_set_name):
    """
    Create a new z/OS data set.
    
    Path Parameters:
        data_set_name: Name of the dataset to create (required)
        
    Query Parameters:
        alcunit: Allocation unit (optional)
        blksize: Block size (optional)
        dirblk: Directory blocks (optional)
        dsorg: Dataset organization (optional)
        primary: Primary space allocation (optional)
        recfm: Record format (optional)
        lrecl: Logical record length (optional)
        dataclass: Data class (optional)
        unit: Unit (optional)
        dsntype: Dataset type (optional)
        mgntclass: Management class (optional)
        dsname: Dataset name (optional)
        avgblk: Average block (optional)
        secondary: Secondary space allocation (optional)
        size: Size (optional)
        storclass: Storage class (optional)
        vol: Volume (optional)
    """
    try:
        print("a\n")
        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400

        attributes = zds.DS_ATTRIBUTES()

        # String parameters
        string_params = ["alcunit", "dsorg", "recfm", "dataclass", "unit", 
                        "dsntype", "mgntclass", "dsname", "storclass", "vol"]
        for param in string_params:
            value = request.args.get(param)
            if value:
                setattr(attributes, param, value)

        # Numeric parameters
        numeric_params = ["blksize", "dirblk", "primary", "lrecl", "avgblk", "secondary", "size"]
        for param in numeric_params:
            value = request.args.get(param)
            if value:
                try:
                    setattr(attributes, param, int(value))
                except ValueError:
                    return jsonify({"error": f"Invalid numeric value for {param}: {value}"}), 400

        zds.create_dataset(data_set_name, attributes)
        
        if "(" in data_set_name and data_set_name.endswith(")"):
            start = data_set_name.find("(")
            end = data_set_name.find(")")
            if start != -1 and end > start:
                member_name = data_set_name[start + 1:end]
                base_dsn = data_set_name[:start]
                response = {
                    "message": f"Data set and/or member created: '{data_set_name}'",
                    "datasetName": base_dsn,
                    "memberName": member_name
                }
            else:
                response = {
                    "message": f"Data set created: '{data_set_name}'",
                    "datasetName": data_set_name
                }
        else:
            response = {
                "message": f"Data set created: '{data_set_name}'",
                "datasetName": data_set_name
            }
        
        return jsonify(response), 201

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not create data set: '{data_set_name}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    

@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>", methods=["DELETE"])
@zds_bp.route("/zosmf/restfiles/ds/-(<volume>)/<path:data_set_name>", methods=["DELETE"])
@zds_bp.route("/zosmf/restfiles/ds/<path:data_set_name>(<member_name>)", methods=["DELETE"])
def delete_dataset(data_set_name, volume=None, member_name=None):
    """
    Delete a z/OS data set or member.
    
    Path Parameters:
        data_set_name: Name of the dataset to delete (required)
        volume: Volume serial (optional)
        member_name: Name of the member to delete (optional)
    """
    try:
        if not data_set_name:
            return jsonify({"error": "data set name is required"}), 400

        warnings_list = []
        if volume:
            warnings_list.append(
                "volume parameter provided but not supported by current C++ function"
            )

        if member_name:
            full_dsn = f"{data_set_name}({member_name})"
        else:
            full_dsn = data_set_name

        zds.delete_dataset(full_dsn)
        
        if member_name:
            response = {
                "message": f"Member '{member_name}' deleted from data set '{data_set_name}'",
                "datasetName": data_set_name,
                "memberName": member_name
            }
        else:
            response = {
                "message": f"Data set '{data_set_name}' deleted",
                "datasetName": data_set_name
            }

        if warnings_list:
            response["warnings"] = warnings_list
        
        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not delete data set: '{full_dsn}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )