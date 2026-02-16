# Zowe Native Protocol SDK

The Zowe Native Protocol SDK is a TypeScript library to interact with the Zowe Native Protocol, enabling developers to interact with mainframe resources directly from their custom Node.js application.

## Building from source

1. From the root of this repository, run `npm install` to install all the dependencies
2. Change to the `packages/sdk` folder and run `npm run build` to build the SDK

The SDK is compiled and saved in the `packages/sdk/lib` folder.

## Usage

Refer to the `test.ts` file in this directory for an example of how to use the SDK.

## API documentation

- **HTML**: `npm run typedoc` — generates HTML in `doc/typedoc`.
- **Markdown (LLM-friendly)**: `npm run typedoc:md` — generates Markdown in `doc/sdk` using [typedoc-plugin-markdown](https://github.com/typedoc2md/typedoc-plugin-markdown). Use this for RAG, context windows, or any LLM consumption; the per-symbol `.md` files are easier to parse than the JSON API dump.
