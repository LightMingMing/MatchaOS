//
// Created by 赵明明 on 2019/11/18.
//

void Start_Kernel()
{
    // Linear Address of Frame Buffer
    int *address = (int *) 0xffff800000a00000;
    int i;

    for (i = 0; i < 1440 * 16; i++)
    {
        *((char *) address + 0) = (char) 0xff;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * 16; i++)
    {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0xff;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * 16; i++)
    {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0xff;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }

    __asm__ __volatile__ ("hlt":: :);
}