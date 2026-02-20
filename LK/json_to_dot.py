import json
import sys
from pathlib import Path


def escape_label(label: str) -> str:
    """Escape quotes and backslashes for Graphviz labels."""
    return label.replace("\\", "\\\\").replace("\"", "\\\"")


def emit_node(f, node, parent_id=None):
    node_id = node.get("id")
    name = node.get("name", "<node>")
    label = escape_label(name)

    f.write(f'  "{node_id}" [label="{label}"];\n')

    if parent_id is not None:
        f.write(f'  "{parent_id}" -> "{node_id}";\n')

    children = node.get("children") or []
    for child in children:
        emit_node(f, child, node_id)


def main():
    if len(sys.argv) < 3:
        print("Usage: python json_to_dot.py <input_json> <output_dot>")
        sys.exit(1)

    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])

    with input_path.open("r", encoding="utf-8") as f:
        root = json.load(f)

    with output_path.open("w", encoding="utf-8") as f:
        f.write("digraph AST {\n")
        f.write("  node [shape=ellipse];\n")
        emit_node(f, root, None)
        f.write("}\n")


if __name__ == "__main__":
    main()

import json

with open("ast.json") as f:
    ast = json.load(f)

dot_lines = ["digraph AST {"]
dot_lines.append("node [shape=box, style=filled, color=lightblue];")
counter = 0

def build_dot(node):
    global counter
    node_id = node["id"]
    label = node["name"]
    dot_lines.append(f'{node_id} [label="{label}"];')
    
    for child in node.get("children", []):
        dot_lines.append(f'{node_id} -> {child["id"]};')
        build_dot(child)

build_dot(ast)
dot_lines.append("}")

with open("ast.dot", "w") as f:
    f.write("\n".join(dot_lines))

print("DOT-file ast.dot generated.")
