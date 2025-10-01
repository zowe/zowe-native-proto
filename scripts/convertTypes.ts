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

// convertTypes.ts - Extract TypeScript interfaces and convert to C structs
import * as ts from "typescript";
import * as fs from "fs";
import * as path from "path";

// Project paths
const SDK_GEN_DIR = path.join(__dirname, "../packages/sdk/src/doc/gen");
const C_TYPES_DIR = path.join(__dirname, "../native/c/types");
const LICENSE_HEADER_PATH = path.join(__dirname, "../LICENSE_HEADER");

// TypeScript to C++ type mapping
const TYPE_MAPPING: Record<string, string> = {
    string: "std::string",
    number: "int",
    boolean: "bool",
    bool: "bool", // Also handle bool directly
    any: "zjson::Value",
    B64String: "std::vector<uint8_t>", // Base64 string -> byte array
};

interface ExtractedInterface {
    name: string;
    properties: Array<{
        name: string;
        type: string;
        optional: boolean;
        comment?: string;
        hasTypeOverride?: boolean;
        isLiteral?: boolean;
    }>;
    extends?: string[];
    sourceFile: string;
}

function mapTypeScriptToCType(tsType: string, optional: boolean): string {
    let baseType: string;

    // Remove "common." prefix from type references
    let cleanType = tsType;
    if (cleanType.includes("common.")) {
        cleanType = cleanType.replace(/common\./g, "");
    }

    // Handle array types
    if (cleanType.endsWith("[]")) {
        const arrayBaseType = cleanType.slice(0, -2);
        const mappedArrayType = TYPE_MAPPING[arrayBaseType] || arrayBaseType;
        baseType = `std::vector<${mappedArrayType}>`;
    }
    // Handle union types with int comments (e.g., "number /* int */")
    else if (cleanType.includes("number") && cleanType.includes("int")) {
        baseType = "int";
    }
    // Handle generic types like { [key: string]: any}
    else if (cleanType.includes("{") && cleanType.includes("}")) {
        baseType = "std::map<std::string, void*>"; // Generic objects as maps
    }
    // Handle quoted literal types like "2.0"
    else if (cleanType.startsWith('"') && cleanType.endsWith('"')) {
        baseType = "std::string";
    }
    // Direct mapping
    else if (TYPE_MAPPING[cleanType]) {
        baseType = TYPE_MAPPING[cleanType];
    }
    // If it's a custom interface/type, assume it's a struct
    else if (cleanType[0].toUpperCase() === cleanType[0]) {
        baseType = cleanType;
    }
    // Default to void pointer for unknown types
    else {
        baseType = "void*";
    }

    // Use zstd::optional for optional parameters, direct types for required ones
    if (optional) {
        return `zstd::optional<${baseType}>`;
    }

    return baseType;
}

function extractInterfaces(filePath: string): ExtractedInterface[] {
    const program = ts.createProgram([filePath], {
        target: ts.ScriptTarget.ES2020,
        module: ts.ModuleKind.CommonJS,
    });
    const checker = program.getTypeChecker();
    const sourceFile = program.getSourceFile(filePath);

    if (!sourceFile) {
        console.warn(`Could not parse file: ${filePath}`);
        return [];
    }

    const interfaces: ExtractedInterface[] = [];
    const fileName = path.basename(filePath, ".ts");

    function visit(node: ts.Node) {
        if (ts.isInterfaceDeclaration(node)) {
            const interfaceName = node.name.text;
            const type = checker.getTypeAtLocation(node);
            const properties: ExtractedInterface["properties"] = [];

            // Extract heritage clauses (extends)
            const extendsClause = node.heritageClauses?.find((clause) => clause.token === ts.SyntaxKind.ExtendsKeyword);
            const extendsTypes = extendsClause?.types.map((type) => type.expression.getText()) || [];

            // Extract properties
            for (const member of node.members) {
                if (ts.isPropertySignature(member) && member.name) {
                    const propName = member.name.getText();
                    const propType = member.type?.getText() || "any";
                    const optional = !!member.questionToken;

                    // Check if this is a literal type (e.g., "listDatasets", 'test', false, 123, "2.0")
                    const isLiteralType =
                        (propType.startsWith('"') && propType.endsWith('"')) ||
                        (propType.startsWith("'") && propType.endsWith("'")) ||
                        propType === "true" ||
                        propType === "false" ||
                        /^\d+$/.test(propType); // numeric literals

                    // Extract comments within the property signature line
                    const memberStart = member.getFullStart();
                    const memberEnd = member.getEnd();
                    const memberText = sourceFile.text.substring(memberStart, memberEnd);

                    let comment: string | undefined;
                    let nameOverride: string | undefined;
                    let typeOverride: string | undefined;

                    // Look for inline comments /* ... */ within the property line
                    const inlineCommentMatch = memberText.match(/\/\*\s*(.*?)\s*\*\//);
                    if (inlineCommentMatch) {
                        comment = inlineCommentMatch[1].trim();

                        // Check for override syntax: name:type, :type, or name:
                        // Split only on the first colon to handle types like zjson::Value
                        const colonIndex = comment.indexOf(":");
                        if (colonIndex !== -1) {
                            const nameMatch = comment.substring(0, colonIndex);
                            const typeMatch = comment.substring(colonIndex + 1);
                            nameOverride = nameMatch.trim() || undefined;
                            typeOverride = typeMatch.trim() || undefined;
                            comment = undefined; // Remove the override syntax from comment
                        }

                        // Only keep non-empty comments that aren't overrides
                        if (!comment) {
                            comment = undefined;
                        }
                    }

                    // Use override values if present, otherwise use extracted values
                    const finalName = nameOverride || propName;
                    const finalType = typeOverride || propType;
                    const hasTypeOverride = typeOverride !== undefined;

                    properties.push({
                        name: finalName,
                        type: finalType,
                        optional,
                        comment,
                        hasTypeOverride,
                        isLiteral: isLiteralType,
                    });
                }
            }

            interfaces.push({
                name: interfaceName,
                properties,
                extends: extendsTypes.length > 0 ? extendsTypes : undefined,
                sourceFile: fileName,
            });
        }

        ts.forEachChild(node, visit);
    }

    visit(sourceFile);
    return interfaces;
}

// Dynamically collected base class properties and interfaces
let baseClassProperties: Map<string, Set<string>> = new Map();
let allInterfaces: Map<string, ExtractedInterface> = new Map();

// License header content
let licenseHeader: string = "";

function loadLicenseHeader(): void {
    try {
        licenseHeader = fs.readFileSync(LICENSE_HEADER_PATH, "utf8");
        // Ensure the license header ends with a newline
        if (!licenseHeader.endsWith("\n")) {
            licenseHeader += "\n";
        }
    } catch (error) {
        console.warn(`Could not load license header from ${LICENSE_HEADER_PATH}:`, error);
        licenseHeader = "";
    }
}

function getInheritedProperties(extendsTypes: string[]): Set<string> {
    const inheritedProps = new Set<string>();

    for (const baseType of extendsTypes) {
        // Extract just the interface name (remove namespace prefixes like "common.")
        const baseName = baseType.includes(".") ? baseType.split(".").pop()! : baseType;
        const baseProps = baseClassProperties.get(baseName);
        if (baseProps) {
            baseProps.forEach((prop) => inheritedProps.add(prop));
        }
    }

    return inheritedProps;
}

function generateCStruct(iface: ExtractedInterface): string {
    const structName = iface.name;
    let header = "";

    // Handle inheritance: first base class uses C++ inheritance, rest are flattened
    let primaryBaseClass: string | undefined;
    let flattenedBases: string[] = [];

    if (iface.extends && iface.extends.length > 0) {
        const baseTypes = iface.extends.map((baseType) => {
            // Extract just the interface name (remove namespace prefixes like "common.")
            return baseType.includes(".") ? baseType.split(".").pop() : baseType;
        });

        primaryBaseClass = baseTypes[0];
        flattenedBases = baseTypes.slice(1); // All bases after the first one

        header += `struct ${structName} : ${primaryBaseClass}\n{\n`;
    } else {
        header += `struct ${structName}\n{\n`;
    }

    // Get inherited properties from primary base class only
    const primaryInheritedProps = primaryBaseClass ? getInheritedProperties([primaryBaseClass]) : new Set<string>();

    // Add nested fields for additional base classes (beyond the first)
    for (const flattenedBase of flattenedBases) {
        // Convert to camelCase: first letter lowercase, rest preserved
        const fieldName = flattenedBase.charAt(0).toLowerCase() + flattenedBase.slice(1);
        header += `    ${flattenedBase} ${fieldName};\n`;
    }

    for (const prop of iface.properties) {
        // Skip properties that are inherited from the primary base class
        if (primaryInheritedProps.has(prop.name)) {
            continue;
        }

        // Skip literal type fields (they are constants, not stored fields)
        // UNLESS they have a type override (user wants to preserve them)
        if (prop.isLiteral && !prop.hasTypeOverride) {
            continue;
        }

        if (prop.comment) {
            header += `    // ${prop.comment}\n`;
        }

        let cType: string;

        if (prop.hasTypeOverride) {
            // Type override from comment: use directly without any inspection or mapping
            cType = prop.type;
            if (prop.optional && !cType.includes("zstd::optional") && !cType.includes("*")) {
                cType = `zstd::optional<${cType}>`;
            }
        } else {
            // Regular TypeScript type: process through mapping
            cType = mapTypeScriptToCType(prop.type, prop.optional);
        }

        header += `    ${cType} ${prop.name};\n`;
    }

    header += `};\n`;

    // Add ZJSON serialization macro
    if (flattenedBases.length > 0) {
        // Use ZJSON_SERIALIZABLE with flatten() for nested fields
        const fieldSpecs: string[] = [];

        // Add flattened fields with .flatten()
        for (const flattenedBase of flattenedBases) {
            // Convert to camelCase: first letter lowercase, rest preserved
            const fieldName = flattenedBase.charAt(0).toLowerCase() + flattenedBase.slice(1);
            fieldSpecs.push(`ZJSON_FIELD(${structName}, ${fieldName}).flatten()`);
        }

        // Add own fields (excluding inherited from primary base)
        const ownFields = iface.properties
            .filter((prop) => {
                // Skip properties that are inherited from the primary base class
                if (primaryInheritedProps.has(prop.name)) {
                    return false;
                }
                return true;
            })
            .map((prop) => `ZJSON_FIELD(${structName}, ${prop.name})`);

        fieldSpecs.push(...ownFields);

        if (fieldSpecs.length > 0) {
            header += `ZJSON_SERIALIZABLE(${structName}, ${fieldSpecs.join(", ")});\n`;
        } else {
            header += `ZJSON_SERIALIZABLE(${structName});\n`;
        }
    } else {
        // Use ZJSON_DERIVE for simple cases without flattening
        // For structs with C++ inheritance, we need to include ALL fields (inherited + own)
        // because ZJSON_DERIVE doesn't automatically handle base class fields
        const allFields: string[] = [];
        const fieldsSeen = new Set<string>();
        const literalFields = new Set<string>();

        // Collect literal field names from child to exclude them (unless they have type override)
        for (const prop of iface.properties) {
            if (prop.isLiteral && !prop.hasTypeOverride) {
                literalFields.add(prop.name);
            }
        }

        // First, add inherited fields from the primary base class (excluding literals without overrides)
        if (primaryBaseClass) {
            const baseInterface = allInterfaces.get(primaryBaseClass);
            if (baseInterface) {
                for (const baseProp of baseInterface.properties) {
                    // Skip if field is overridden with literal in child, or already seen
                    if (!fieldsSeen.has(baseProp.name) && !literalFields.has(baseProp.name)) {
                        allFields.push(baseProp.name);
                        fieldsSeen.add(baseProp.name);
                    }
                }
            }
        }

        // Then add own fields (skip if already added from base or is literal without override)
        for (const prop of iface.properties) {
            const shouldSkip = prop.isLiteral && !prop.hasTypeOverride;
            if (!fieldsSeen.has(prop.name) && !shouldSkip) {
                allFields.push(prop.name);
                fieldsSeen.add(prop.name);
            }
        }

        if (allFields.length > 0) {
            header += `ZJSON_DERIVE(${structName}, ${allFields.join(", ")});\n`;
        }
    }

    header += `\n`;
    return header;
}

function generateHeaderFile(interfaces: ExtractedInterface[], fileName: string): string {
    const headerGuard = `${fileName.toUpperCase()}_TYPES_H`;
    let content = "";

    // Add license header if available
    if (licenseHeader) {
        content += licenseHeader + "\n";
    }

    // Add auto-generation disclaimer
    content += `/*\n`;
    content += ` * AUTO-GENERATED - DO NOT EDIT\n`;
    content += ` * Generated from ${fileName}.ts - edit there instead\n`;
    content += ` */\n\n`;

    content += `#ifndef ${headerGuard}\n#define ${headerGuard}\n\n`;
    content += `#include <string>\n`;
    content += `#include <vector>\n`;
    content += `#include <map>\n`;
    content += `#include <cstdint>\n`; // for uint8_t
    content += `#include "../zstd.hpp"\n`; // for zstd::optional
    content += `#define ZJSON_ENABLE_STRUCT_SUPPORT\n`; // Enable struct serialization macros
    content += `#include "../zjson.hpp"\n`; // for ZJSON_DERIVE macro

    // Include common.h for non-common files that might inherit from common types
    if (fileName !== "common") {
        content += `#include "common.h"\n`;
    }

    content += `\n// Generated C++ structs from ${fileName}.ts\n\n`;

    for (const iface of interfaces) {
        content += generateCStruct(iface);
    }

    content += `#endif\n`;
    return content;
}

function collectBaseClassProperties(interfaces: ExtractedInterface[]): void {
    // First pass: collect all interface properties and store interfaces
    for (const iface of interfaces) {
        const props = new Set<string>();
        for (const prop of iface.properties) {
            props.add(prop.name);
        }
        baseClassProperties.set(iface.name, props);
        allInterfaces.set(iface.name, iface);
    }

    // Second pass: add inherited properties to base classes
    for (const iface of interfaces) {
        if (iface.extends && iface.extends.length > 0) {
            const currentProps = baseClassProperties.get(iface.name)!;

            for (const baseType of iface.extends) {
                const baseName = baseType.includes(".") ? baseType.split(".").pop()! : baseType;
                const baseProps = baseClassProperties.get(baseName);
                if (baseProps) {
                    baseProps.forEach((prop) => currentProps.add(prop));
                }
            }
        }
    }
}

function processAllGenFiles(): void {
    // Load the license header
    loadLicenseHeader();

    if (!fs.existsSync(C_TYPES_DIR)) {
        fs.mkdirSync(C_TYPES_DIR, { recursive: true });
    }

    const genFiles = fs.readdirSync(SDK_GEN_DIR).filter((file) => file.endsWith(".ts"));
    console.log(`Processing ${genFiles.length} TypeScript files...`);

    // First pass: extract all interfaces from all files
    const allInterfaces: ExtractedInterface[] = [];
    const fileInterfaceMap: Map<string, ExtractedInterface[]> = new Map();

    for (const file of genFiles) {
        const filePath = path.join(SDK_GEN_DIR, file);
        const fileName = path.basename(file, ".ts");

        console.log(`Extracting interfaces from ${file}...`);
        const interfaces = extractInterfaces(filePath);

        allInterfaces.push(...interfaces);
        fileInterfaceMap.set(fileName, interfaces);
    }

    // Build the base class property map
    collectBaseClassProperties(allInterfaces);

    // Second pass: generate header files
    for (const [fileName, interfaces] of fileInterfaceMap) {
        if (interfaces.length > 0) {
            const headerContent = generateHeaderFile(interfaces, fileName);
            const headerPath = path.join(C_TYPES_DIR, `${fileName}.h`);

            fs.writeFileSync(headerPath, headerContent);
            console.log(`Generated ${headerPath} with ${interfaces.length} interfaces`);
        } else {
            console.log(`No interfaces found in ${fileName}.ts`);
        }
    }
}

// Run the conversion
if (require.main === module) {
    try {
        processAllGenFiles();
        console.log("Type conversion completed successfully!");
    } catch (error) {
        console.error("Error during type conversion:", error);
        process.exit(1);
    }
}
