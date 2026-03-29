import os
import re

FILES_TO_MERGE = [  'core/sw/inc/constants.h', 
                    'core/sw/inc/vga_driver.h',  'core/sw/src/vga_driver.c',
                    'core/sw/inc/ps2_input.h',  'core/sw/src/ps2_input.c',  
                    'core/sw/inc/io.h', 'core/sw/src/io.c',
                    'core/sw/inc/test_la_c.h',  'core/sw/src/test_la_c.c', 
                    'core/sw/inc/visualizer_logic.h', 'core/sw/src/visualizer_logic.c', 
                    'core/sw/inc/draw_screen.h', 'core/sw/src/draw_screen.c',
                    'core/sw/inc/interface.h', 'core/sw/src/interface.c',
                    'core/sw/src/main.c'] 
OUTPUT_FILE = 'combined_cpulator.c'

def merge_files():
    combined_content = []
    local_include_re = re.compile(r'^#include\s+"[^"]+"', re.MULTILINE)
    # Regex to find 'static inline' regardless of line start
    static_inline_re = re.compile(r'static\s+inline\s+', re.MULTILINE)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, OUTPUT_FILE)

    for filename in FILES_TO_MERGE:
        filepath = os.path.join(script_dir, filename)
        
        if not os.path.exists(filepath):
            print(f"Warning: {filename} not found. Skipping.")
            continue
            
        with open(filepath, 'r') as f:
            content = f.read()
            
            # Remove local includes and all instances of static inline
            content = local_include_re.sub(f"// Removed include: {filename}", content)
            content = static_inline_re.sub("", content)
            
            # Hardcode specific hardware macros for CPulator
            content = content.replace("PIXEL_BUF_CTRL_BASE", "0xFF203020")
            content = content.replace("FPGA_CHAR_BASE", "0x09000000")
            content = content.replace("LEDR_BASE", "0xFF200000")
            content = content.replace("PS2_BASE", "0xFF200100")
            content = content.replace("HEX3_HEX0_BASE", "0xFF200020")
            content = content.replace("HEX5_HEX4_BASE", "0xFF200030")
            
            combined_content.append(f"/* --- START OF {filename} --- */")
            combined_content.append(content)
            combined_content.append(f"/* --- END OF {filename} --- */\n")

    # Final string manipulation for relocation
    full_text = "\n".join(combined_content)
    
    target_proto = "void put_on_leds(uint32_t led_val);"
    anchor_proto = "void draw_trigger_marker(const ZoomState* state, uint32_t trigger_position);"

    if target_proto in full_text and anchor_proto in full_text:
        # Remove the first instance of the target
        full_text = full_text.replace(target_proto, "", 1)
        # Find the anchor and insert the target on the next line
        insertion_idx = full_text.find(anchor_proto) + len(anchor_proto)
        full_text = (
            full_text[:insertion_idx] + 
            "\n" + target_proto + 
            full_text[insertion_idx:]
        )

    with open(output_path, 'w') as f:
        f.write(full_text)
    
    print(f"Success! Created {OUTPUT_FILE} for CPulator.")

if __name__ == "__main__":
    merge_files()