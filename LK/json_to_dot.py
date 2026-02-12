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
