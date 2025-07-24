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

zusf_bp = Blueprint('zusf', __name__, url_prefix='/pythonservice')


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
    
@zusf_bp.route("/zosmf/restfiles/fs/<path:filepath_name>", methods=["PUT"])
def write_uss_file(filepath_name):
    """
    Write data to a USS (Unix System Services) file.

    This endpoint calls the zusf.write_uss_file function and formats the output similar to the C++ CLI.

    Path Parameters:
        filepath_name: USS file path to write to (required)

    Query Parameters:
        encoding: Encoding/codepage for text conversion (optional)
        etag: ETag for conditional write (optional)
        etag-only: Return only ETag and created status (optional, default: false)

    Request Body:
        The data to write to the file (text or binary)
        Can be JSON with "records" field or raw text/binary data
    """
    try:
        if not filepath_name:
            return jsonify({"error": "file path is required"}), 400

        if not filepath_name.startswith("/"):
            filepath_name = "/" + filepath_name

        # Get query parameters
        encoding = request.args.get("encoding", "")
        etag = request.args.get("etag", "")
        etag_only = request.args.get("etag-only", "false").lower()

        # Get the data from request body
        if request.is_json:
            # Handle JSON request body
            json_data = request.get_json()
            if "records" in json_data:
                if isinstance(json_data["records"], list):
                    # Handle bytes format - convert list of integers to string
                    data = ''.join(chr(b) for b in json_data["records"])
                else:
                    # Handle text format
                    data = str(json_data["records"])
            elif "data" in json_data:
                data = str(json_data["data"])
            else:
                # Handle case where entire JSON is the data
                data = str(json_data)
        else:
            # Handle raw text/binary data (like CLI reading from stdin)
            data = request.get_data(as_text=True)

        if not data:
            return jsonify({"error": "no data provided to write"}), 400

        # Call the C++ function
        returned_etag = zusf.write_uss_file(filepath_name, data, encoding, etag)

        # Format response based on etag-only parameter
        if etag_only == "true":
            response = {
                "etag": returned_etag,
                "created": True  # Note: C++ function doesn't expose created flag from zusf.created
            }
        else:
            response = {
                "message": f"Wrote data to '{filepath_name}'",
                "filePath": filepath_name,
                "etag": returned_etag,
                "success": True
            }

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"could not write to USS file: '{filepath_name}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    
def convert_symbolic_to_octal(mode_str):
    """
    Convert symbolic permission string (e.g., 'rwxr--r--') to octal string (e.g., '644').
    
    Args:
        mode_str: 9-character string with r/w/x/- characters
        
    Returns:
        3-character octal string
    """
    if not mode_str or len(mode_str) != 9:
        raise ValueError("Mode must be exactly 9 characters (rwxrwxrwx format)")
    
    octal_digits = []
    
    # Process in groups of 3 (owner, group, other)
    for i in range(0, 9, 3):
        group = mode_str[i:i+3]
        value = 0
        
        # Read permission (4)
        if group[0] == 'r':
            value += 4
        elif group[0] != '-':
            raise ValueError(f"Invalid character '{group[0]}' at position {i}. Expected 'r' or '-'")
            
        # Write permission (2)
        if group[1] == 'w':
            value += 2
        elif group[1] != '-':
            raise ValueError(f"Invalid character '{group[1]}' at position {i+1}. Expected 'w' or '-'")
            
        # Execute permission (1)
        if group[2] == 'x':
            value += 1
        elif group[2] != '-':
            raise ValueError(f"Invalid character '{group[2]}' at position {i+2}. Expected 'x' or '-'")
            
        octal_digits.append(str(value))
    
    return ''.join(octal_digits)


@zusf_bp.route("/zosmf/restfiles/fs/<path:file_path_name>", methods=["POST"])
def create_uss_file_or_dir(file_path_name):
    """
    Create a USS (Unix System Services) file or directory.

    This endpoint calls the zusf.create_uss_file or zusf.create_uss_dir function 
    based on the type specified in the request body.

    Path Parameters:
        file_path_name: USS file or directory path to create (required)

    Request Body (JSON):
        type: "file" or "directory" (required)
        mode: File/directory permissions in symbolic (rwxr--r--) or octal (644) format 
              (optional, defaults: file="rw-r--r--", directory="rwxr-xr-x")
    """
    try:
        if not file_path_name:
            return jsonify({"error": "file path is required"}), 400

        if not file_path_name.startswith("/"):
            file_path_name = "/" + file_path_name

        # Get request body
        if not request.is_json:
            return jsonify({"error": "Content-Type must be application/json"}), 400

        json_data = request.get_json()
        if not json_data:
            return jsonify({"error": "Request body is required"}), 400

        # Get type (file or directory)
        item_type = json_data.get("type", "").lower()
        if item_type not in ["file", "directory"]:
            return jsonify({"error": "type must be 'file' or 'directory'"}), 400

        # Get mode with appropriate defaults
        if item_type == "file":
            default_mode = "rw-r--r--"  # 644 equivalent
        else:  # directory
            default_mode = "rwxr-xr-x"  # 755 equivalent
            
        mode_input = json_data.get("mode", default_mode)
        
        # Convert symbolic mode to octal if needed
        try:
            if len(mode_input) == 9 and any(c in mode_input for c in 'rwx-'):
                # Symbolic format (e.g., "rwxr--r--")
                mode = convert_symbolic_to_octal(mode_input)
            elif len(mode_input) == 3 and mode_input.isdigit():
                # Already in octal format (e.g., "644")
                mode = mode_input
            else:
                return jsonify({
                    "error": "Invalid mode format. Use symbolic (e.g., 'rwxr--r--') or octal (e.g., '644')"
                }), 400
        except ValueError as ve:
            return jsonify({"error": f"Invalid mode: {str(ve)}"}), 400

        # Call appropriate C++ function
        if item_type == "file":
            zusf.create_uss_file(file_path_name, mode)
            message = f"USS file '{file_path_name}' created"
        else:  # directory
            zusf.create_uss_dir(file_path_name, mode)
            message = f"USS directory '{file_path_name}' created"

        response = {
            "message": message,
            "path": file_path_name,
            "type": item_type,
            "mode": mode,
            "originalMode": mode_input,
            "success": True
        }

        return jsonify(response), 201

    except Exception as e:
        error_msg = str(e)
        item_type = json_data.get("type", "file/directory") if 'json_data' in locals() else "file/directory"
        return (
            jsonify(
                {
                    "error": f"could not create USS {item_type}: '{file_path_name}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    
@zusf_bp.route("/zosmf/restfiles/fs/<path:file_pathname>", methods=["DELETE"])
def delete_uss_item(file_pathname):
    """
    Delete a USS (Unix System Services) file or directory.

    This endpoint calls the zusf.delete_uss_item function to delete files or directories.

    Path Parameters:
        file_pathname: USS file or directory path to delete (required)

    Query Parameters:
        recursive: Delete directories recursively (optional, default: false)
    """
    try:
        if not file_pathname:
            return jsonify({"error": "file path is required"}), 400

        if not file_pathname.startswith("/"):
            file_pathname = "/" + file_pathname

        # Get recursive parameter
        recursive = request.args.get("recursive", "false").lower() == "true"

        # Call the C++ function
        zusf.delete_uss_item(file_pathname, recursive)

        response = {
            "message": f"USS item '{file_pathname}' deleted successfully",
            "path": file_pathname,
            "recursive": recursive,
            "success": True
        }

        return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        return (
            jsonify(
                {
                    "error": f"Failed to delete USS item '{file_pathname}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    
@zusf_bp.route("/zosmf/restfiles/fs/<path:file_path_name>", methods=["PUT"])
def uss_file_utilities(file_path_name):
    """
    Perform utilities operations on USS (Unix System Services) files/directories.
    
    Supports chmod, chown, and chtag operations based on the request body.
    
    Path Parameters:
        file_path_name: USS file or directory path (required)
        
    Request Body (JSON):
        For chmod:
            {
                "request": "chmod",
                "mode": "rwxr--r--" or "755",
                "recursive": true/false (optional, default: false)
            }
            
        For chown:
            {
                "request": "chown", 
                "owner": "username",
                "recursive": true/false (optional, default: false)
            }
            
        For chtag:
            {
                "request": "chtag",
                "tag": "tag_value", 
                "recursive": true/false (optional, default: false)
            }
            
        For file content write (existing functionality):
            {
                "records": "file content data"
            }
    """
    try:
        if not file_path_name:
            return jsonify({"error": "file path is required"}), 400

        if not file_path_name.startswith("/"):
            file_path_name = "/" + file_path_name

        # Get request body
        if not request.is_json:
            return jsonify({"error": "Content-Type must be application/json"}), 400

        json_data = request.get_json()
        if not json_data:
            return jsonify({"error": "Request body is required"}), 400

        # Check if this is a utilities request or file write request
        request_type = json_data.get("request", "").lower()
        
        if request_type in ["chmod", "chown", "chtag"]:
            # Handle utilities operations
            recursive = json_data.get("recursive", False)
            
            if request_type == "chmod":
                mode = json_data.get("mode", "")
                if not mode:
                    return jsonify({"error": "mode is required for chmod operation"}), 400
                
                # Convert symbolic mode to octal if needed (reuse existing function)
                try:
                    if len(mode) == 9 and any(c in mode for c in 'rwx-'):
                        # Symbolic format
                        octal_mode = convert_symbolic_to_octal(mode)
                    elif len(mode) == 3 and mode.isdigit():
                        # Already in octal format
                        octal_mode = mode
                    else:
                        return jsonify({
                            "error": "Invalid mode format. Use symbolic (e.g., 'rwxr--r--') or octal (e.g., '644')"
                        }), 400
                except ValueError as ve:
                    return jsonify({"error": f"Invalid mode: {str(ve)}"}), 400
                
                # Call chmod function
                zusf.chmod_uss_item(file_path_name, octal_mode, recursive)
                
                response = {
                    "message": f"USS path '{file_path_name}' modified: '{mode}'",
                    "path": file_path_name,
                    "operation": "chmod",
                    "mode": octal_mode,
                    "originalMode": mode,
                    "recursive": recursive,
                    "success": True
                }
                
            elif request_type == "chown":
                owner = json_data.get("owner", "")
                if not owner:
                    return jsonify({"error": "owner is required for chown operation"}), 400
                
                # Call chown function
                zusf.chown_uss_item(file_path_name, owner, recursive)
                
                response = {
                    "message": f"USS path '{file_path_name}' ownership changed to '{owner}'",
                    "path": file_path_name,
                    "operation": "chown", 
                    "owner": owner,
                    "recursive": recursive,
                    "success": True
                }
                
            elif request_type == "chtag":
                tag = json_data.get("tag", "")
                if not tag:
                    return jsonify({"error": "tag is required for chtag operation"}), 400
                
                # Call chtag function
                zusf.chtag_uss_item(file_path_name, tag, recursive)
                
                response = {
                    "message": f"USS path '{file_path_name}' tagged with '{tag}'",
                    "path": file_path_name,
                    "operation": "chtag",
                    "tag": tag,
                    "recursive": recursive,
                    "success": True
                }
                
            return jsonify(response)
            
        else:
            # Handle file write operation (existing functionality)
            encoding = request.args.get("encoding", "")
            etag = request.args.get("etag", "")
            etag_only = request.args.get("etag-only", "false").lower()

            # Get the data from request body
            if "records" in json_data:
                if isinstance(json_data["records"], list):
                    # Handle bytes format - convert list of integers to string
                    data = ''.join(chr(b) for b in json_data["records"])
                else:
                    # Handle text format
                    data = str(json_data["records"])
            elif "data" in json_data:
                data = str(json_data["data"])
            else:
                return jsonify({"error": "no data provided to write and no valid utility request specified"}), 400

            if not data:
                return jsonify({"error": "no data provided to write"}), 400

            # Call the C++ function for file write
            returned_etag = zusf.write_uss_file(file_path_name, data, encoding, etag)

            # Format response based on etag-only parameter
            if etag_only == "true":
                response = {
                    "etag": returned_etag,
                    "created": True
                }
            else:
                response = {
                    "message": f"Wrote data to '{file_path_name}'",
                    "filePath": file_path_name,
                    "etag": returned_etag,
                    "success": True
                }

            return jsonify(response)

    except Exception as e:
        error_msg = str(e)
        operation = json_data.get("request", "file operation") if 'json_data' in locals() else "file operation"
        return (
            jsonify(
                {
                    "error": f"could not perform {operation} on USS path: '{file_path_name}' - {error_msg}",
                    "details": error_msg,
                }
            ),
            500,
        )
    
