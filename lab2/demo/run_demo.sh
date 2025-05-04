current_dir=$(pwd)

../build/frontend \
    --mode DFA \
    --general-config ${current_dir}/demo_config.yml \
    --backend-config ${current_dir}/demo_config.yml \
    --input-file ${current_dir}/demo_input.txt \
    --output-file ${current_dir}/demo_output_dfa.txt \

../build/frontend \
    --mode FLEX \
    --general-config ${current_dir}/demo_config.yml \
    --backend-config ${current_dir}/demo_config.yml \
    --input-file ${current_dir}/demo_input.txt \
    --output-file ${current_dir}/demo_output_flex.txt \