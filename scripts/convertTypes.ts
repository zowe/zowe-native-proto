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
    any: "void*",
    B64String: "std::vector<uint8_t>", // Base64 string -> byte array
};

interface ExtractedInterface {
    name: string;
    properties: Array<{
        name: string;
        type: string;
        optional: boolean;
        comment?: string;
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

    // Use pointers for optional parameters, direct types for required ones
    if (optional && baseType !== "void*" && !baseType.startsWith("std::vector") && !baseType.startsWith("std::map")) {
        return `${baseType}*`;
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

                    // Extract comments within the property signature line
                    const memberStart = member.getFullStart();
                    const memberEnd = member.getEnd();
                    const memberText = sourceFile.text.substring(memberStart, memberEnd);

                    let comment: string | undefined;
                    let nameOverride: string | undefined;
                    let typeOverride: string | undefined;

                    // Look for inline comments /* ... */ within the property line
                    const inlineCommentMatch = memberText.match(/\/\*\s*([^*]+)\s*\*\//);
                    if (inlineCommentMatch) {
                        comment = inlineCommentMatch[1].trim();

                        // Check for override syntax: name:type, :type, or name:
                        const overrideMatch = comment?.match(/^([^:]*):([^:]*)$/);
                        if (overrideMatch) {
                            const [, nameMatch, typeMatch] = overrideMatch;
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

                    properties.push({
                        name: finalName,
                        type: finalType,
                        optional,
                        comment,
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

// Dynamically collected base class properties
let baseClassProperties: Map<string, Set<string>> = new Map();

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

    // Handle inheritance with struct inheritance syntax
    if (iface.extends && iface.extends.length > 0) {
        const baseTypes = iface.extends.map((baseType) => {
            // Extract just the interface name (remove namespace prefixes like "common.")
            return baseType.includes(".") ? baseType.split(".").pop() : baseType;
        });
        header += `struct ${structName} : ${baseTypes.map((base) => `public ${base}`).join(", ")} {\n`;
    } else {
        header += `struct ${structName} {\n`;
    }

    // Add properties, but exclude ones that are inherited from base classes
    const hasInheritance = iface.extends && iface.extends.length > 0;
    const inheritedProps = hasInheritance ? getInheritedProperties(iface.extends!) : new Set<string>();

    for (const prop of iface.properties) {
        // Skip properties that are inherited from base classes
        if (hasInheritance && inheritedProps.has(prop.name)) {
            continue;
        }

        if (prop.comment) {
            header += `    // ${prop.comment}\n`;
        }

        // For type overrides, use them directly if they look like C++ types,
        // otherwise process through the mapping function
        let cType: string;
        if (
            prop.type.includes("*") ||
            prop.type.includes("std::") ||
            prop.type.includes("int") ||
            prop.type.includes("char") ||
            prop.type === "bool" // Only exact match for bool, not boolean
        ) {
            // This looks like a C++ type override, use directly
            cType = prop.type;
            if (prop.optional && !cType.includes("*")) {
                cType += "*"; // Add pointer for optional
            }
        } else {
            // Regular TypeScript type, process through mapping
            cType = mapTypeScriptToCType(prop.type, prop.optional);
        }

        const suffix = prop.optional ? "; // optional" : ";";
        header += `    ${cType} ${prop.name}${suffix}\n`;
    }

    header += `};\n\n`;
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

function collectBaseClassProperties(allInterfaces: ExtractedInterface[]): void {
    // First pass: collect all interface properties
    for (const iface of allInterfaces) {
        const props = new Set<string>();
        for (const prop of iface.properties) {
            props.add(prop.name);
        }
        baseClassProperties.set(iface.name, props);
    }

    // Second pass: add inherited properties to base classes
    for (const iface of allInterfaces) {
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
