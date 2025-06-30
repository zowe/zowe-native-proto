"""
zUSF (z/OS Unix System Files) related routes for the Flask application.
Handles USS file listing and reading operations.
"""

import json
import hashlib
import time
from flask import Blueprint, jsonify, request
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from bindings import zusf_py as zusf

zusf_bp = Blueprint('zusf', __name__)


@zusf_bp.route("/zosmf/restfiles/fs", methods=["GET"])
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
            # Fallback to simple line-by-line parsing
            lines = uss_output.strip().split("\n") if uss_output else []
            items = []
            for line in lines:
                if line.strip():
                    # Basic parsing for non-JSON output
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


@zusf_bp.route("/zosmf/restfiles/fs/<path:file_path>", methods=["GET"])
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
            # Convert string to bytes array
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