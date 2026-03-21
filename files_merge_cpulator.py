import os
import re

FILES_TO_MERGE = ['core/sw/visualizer_logic.h', 'core/sw/visualizer_logic.c', 'core/sw/draw_screen.h', 'core/sw/vga_driver.h',  
                  'core/sw/vga_driver.c', 'core/sw/draw_screen.c', 'core/sw/main.c']
OUTPUT_FILE = 'combined_cpulator.c'

def merge_files():
    combined_content = []
    local_include_re = re.compile(r'^#include\s+"[^"]+"', re.MULTILINE)
    static_re = re.compile(r'^static\s+', re.MULTILINE)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, OUTPUT_FILE)

    for filename in FILES_TO_MERGE:
        filepath = os.path.join(script_dir, filename)
        
        if not os.path.exists(filepath):
            print(f"Warning: {filename} not found. Skipping.")
            continue
            
        with open(filepath, 'r') as f:
            content = f.read()
            
            # Remove local includes and static keywords
            content = local_include_re.sub(f"// Removed include: {filename}", content)
            content = static_re.sub("", content)
            
            # Hardcode specific hardware macros for CPulator
            content = content.replace("PIXEL_BUF_CTRL_BASE", "0xFF203020")
            
            combined_content.append(f"/* --- START OF {filename} --- */")
            combined_content.append(content)
            combined_content.append(f"/* --- END OF {filename} --- */\n")

    with open(output_path, 'w') as f:
        f.write("\n".join(combined_content))
    
    print(f"Success! Copy the contents of {OUTPUT_FILE} into CPulator.")

if __name__ == "__main__":
    merge_files()