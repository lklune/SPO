import json
import sys
from pathlib import Path


def escape_label(label: str) -> str:
    """Escape quotes, backslashes and special chars for Graphviz labels."""
    if not label:
        return "<empty>"
    
    label = label.replace("\\", "\\\\")
    label = label.replace("\"", "\\\"")
    label = label.replace("\n", "\\n")
    label = label.replace("\r", "\\r")
    label = label.replace("\t", "\\t")
    
    # невалидные UTF-8 символы
    label = label.encode('utf-8', 'ignore').decode('utf-8')
    
    return label


def emit_node(f, node, parent_id=None):
    """Рекурсивно генерируем узлы DOT."""
    node_id = node.get("id", "unknown")
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

    try:
        with input_path.open("r", encoding="utf-8", errors="ignore") as f:
            root = json.load(f)
    except json.JSONDecodeError as e:
        print(f"JSON parsing error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading file: {e}")
        sys.exit(1)

    try:
        with output_path.open("w", encoding="utf-8") as f:
            f.write("digraph AST {\n")
            f.write("  node [shape=ellipse, fontname=\"Arial\"];\n")
            f.write("  rankdir=TB;\n")
            emit_node(f, root, None)
            f.write("}\n")
        
        print(f"✓ DOT file generated: {output_path}")
        print(f"  Usage: dot -Tpng {output_path} -o {output_path.stem}.png")
    
    except Exception as e:
        print(f"Error writing file: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()