#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&(id_dest->val));
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  if (id_dest->type == OP_TYPE_REG) rtl_sr(id_dest->reg, id_dest->width, &t0);
  else if (id_dest->type == OP_TYPE_MEM) rtl_sm(&id_dest->addr, id_dest->width, &t0);
  else assert(0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
    t0 = cpu.esp;
    if(decoding.is_operand_size_16) {
        rtl_lr_w(&t1, R_AX); rtl_push(&t1);
        rtl_lr_w(&t1, R_CX); rtl_push(&t1);
        rtl_lr_w(&t1, R_DX); rtl_push(&t1);
        rtl_lr_w(&t1, R_BX); rtl_push(&t1);
        rtl_push(&t0);
        rtl_lr_w(&t1, R_BP); rtl_push(&t1);
        rtl_lr_w(&t1, R_SI); rtl_push(&t1);
        rtl_lr_w(&t1, R_DI); rtl_push(&t1);
        
    }
    else {
        rtl_lr_l(&t1, R_EAX); rtl_push(&t1);
        rtl_lr_l(&t1, R_ECX); rtl_push(&t1);
        rtl_lr_l(&t1, R_EDX); rtl_push(&t1);
        rtl_lr_l(&t1, R_EBX); rtl_push(&t1);
        rtl_push(&t0);
        rtl_lr_l(&t1, R_EBP); rtl_push(&t1);
        rtl_lr_l(&t1, R_ESI); rtl_push(&t1);
        rtl_lr_l(&t1, R_EDI); rtl_push(&t1);
    }
    print_asm("pusha");
}

make_EHelper(popa) {
    if(decoding.is_operand_size_16) {
        rtl_pop(&t1); rtl_sr_w(R_DI, &t1);
        rtl_pop(&t1); rtl_sr_w(R_SI, &t1);
        rtl_pop(&t1); rtl_sr_w(R_BP, &t1);
        rtl_pop(&t1);
        rtl_pop(&t1); rtl_sr_w(R_BX, &t1);
        rtl_pop(&t1); rtl_sr_w(R_DX, &t1);
        rtl_pop(&t1); rtl_sr_w(R_CX, &t1);
        rtl_pop(&t1); rtl_sr_w(R_AX, &t1);
    }
    else {
        rtl_pop(&t1); rtl_sr_l(R_EDI, &t1);
        rtl_pop(&t1); rtl_sr_l(R_ESI, &t1);
        rtl_pop(&t1); rtl_sr_l(R_EBP, &t1);
        rtl_pop(&t1);
        rtl_pop(&t1); rtl_sr_l(R_EBX, &t1);
        rtl_pop(&t1); rtl_sr_l(R_EDX, &t1);
        rtl_pop(&t1); rtl_sr_l(R_ECX, &t1);
        rtl_pop(&t1); rtl_sr_l(R_EAX, &t1);
    }
    print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
    if (decoding.is_operand_size_16) {
        rtl_lr_w(&t0, R_AX); // 取出 AX 放到t0里
        rtl_sext(&t0, &t0, 2); // 字扩展
        rtl_sari(&t0, &t0, 16); // 右移16位
        rtl_sr_w(R_DX, &t0); // 把高16位放到DX里
    } else {
  	    rtl_lr_l(&t0, R_EAX);
  	    rtl_sari(&t0, &t0, 31);
//   	    rtl_sext(&t0, &t0, 4);
        rtl_sr_l(R_EDX, &t0);
    }
    print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
      rtl_lr_b(&t0, R_AL); // 取出 AL 放到t0里
  	  rtl_sext(&t0, &t0, 1);
      rtl_sr_w(R_AX, &t0); // 不能使用 reg_w(R_AX) = t0; 否则coremark跑分时打开diff-test会出问题
  }
  else {
      rtl_lr_w(&t0, R_AX); // 取出 AX 放到t0里
  	  rtl_sext(&t0, &t0, 2);
      rtl_sr_l(R_EAX, &t0); // 不能使用 reg_l(R_EAX) = t0; 否则coremark跑分时打开diff-test会出问题
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
