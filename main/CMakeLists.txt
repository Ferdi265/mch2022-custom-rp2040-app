# This example code is in the Public Domain (or CC0 licensed, at your option.)
# Unless required by applicable law or agreed to in writing, this
# software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, either express or implied.

idf_component_register(
    SRCS
        "main.c"
        "rp2040_custom.c"
    INCLUDE_DIRS
        "." "include"
    EMBED_FILES
        ${project_dir}/resources/rpi_logo.png
        ${project_dir}/resources/rp2040_custom.bin
)
