/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
 */

#include <cstdio>
#include <iostream>
#include <optional>
#include <string>

#include "zjson.hpp"

// JSON-RPC 2.0 request/response types using zjson serialization,
// following the same pattern as the zowed backend (see native/c/server/rpcio.hpp).

struct RpcRequest
{
    std::string jsonrpc;
    std::string method;
    std::optional<zjson::Value> params;
    int id;
};
ZJSON_DERIVE(RpcRequest, jsonrpc, method, params, id);

struct ErrorDetails
{
    int code;
    std::string message;
};
ZJSON_DERIVE(ErrorDetails, code, message);

struct RpcResponse
{
    std::string jsonrpc;
    std::optional<zjson::Value> result;
    std::optional<ErrorDetails> error;
    int id;
};
ZJSON_SERIALIZABLE(RpcResponse,
                   ZJSON_FIELD(RpcResponse, jsonrpc),
                   ZJSON_FIELD(RpcResponse, result).skip_serializing_if_none(),
                   ZJSON_FIELD(RpcResponse, error).skip_serializing_if_none(),
                   ZJSON_FIELD(RpcResponse, id));

// Application-level types

struct AddParams
{
    int a;
    int b;
};
ZJSON_DERIVE(AddParams, a, b);

struct AddResult
{
    int sum;
};
ZJSON_DERIVE(AddResult, sum);

static void send_response(const RpcResponse &response)
{
    auto json = zjson::to_string(response);
    if (json.has_value())
        std::cout << json.value() << std::endl;
}

static void send_error(int id, int code, const std::string &message)
{
    RpcResponse response;
    response.jsonrpc = "2.0";
    response.error = ErrorDetails{code, message};
    response.id = id;
    send_response(response);
}

static void handle_add(const RpcRequest &request)
{
    if (!request.params.has_value())
    {
        send_error(request.id, -32602, "Invalid parameters");
        return;
    }

    auto params = zjson::from_value<AddParams>(request.params.value());
    if (!params.has_value())
    {
        send_error(request.id, -32602, "Invalid parameters");
        return;
    }

    AddResult add_result;
    add_result.sum = params.value().a + params.value().b;

    RpcResponse response;
    response.jsonrpc = "2.0";
    response.result = zjson::Serializable<AddResult>::serialize(add_result);
    response.id = request.id;
    send_response(response);
}

int main()
{
    std::fprintf(stderr, "JSON-RPC server started...\n");

    std::string line;
    while (std::getline(std::cin, line))
    {
        if (line.empty())
            continue;

        auto parsed = zjson::from_str<RpcRequest>(line);
        if (!parsed.has_value())
        {
            send_error(0, -32700, "Parse error");
            continue;
        }

        const RpcRequest &request = parsed.value();

        if (request.method == "Arith.Add")
        {
            handle_add(request);
        }
        else
        {
            send_error(request.id, -32601, "Method not found");
        }
    }

    std::fprintf(stderr, "JSON-RPC server stopped.\n");
    return 0;
}
