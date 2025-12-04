  if (current->expression_tree)
        {
            // reset the temporary register counter for each new statement
            next_temp_register = 8;

            // generate all the MIPS for the expression
            // the final result will be in the register returned by this call
            int final_result_reg = generate_mips_for_ast(output_file, current->expression_tree);

            // store final result from the temp reg into the variable's memory
            if (dst->data_type == TYPE_INT)
            {
                fprintf(output_file, "    sd r%d, %s(r0)\n", final_result_reg, dst->id);
                // printf("    sd r%d, %s(r0)\n", final_
                fprintf(output_file, "    sd r%d, %s(r0)\n", final_result_reg, dst->id);
                // printf("    sd r%d, %s(r0)\n", final_result_reg, dst->id);
            }
            else
            {
                fprintf(output_file, "    sb r%d, %s(r0)\n", final_result_reg, dst->id);
                // printf("    sb r%d, %s(r0)\n", final_result_reg, dst->id);
            }
            fprintf(output_file, "\n");
        }