# Type Sizes

Print the types, sizes and smallest and largest number (in dec).  

## Usage

```
# sudo insmod print_my_types.ko

# dmesg
    ...
    [ 2818.303369] --- types ---
    [ 2818.303391] unsigned short [2]: 0 -> 65535
    [ 2818.303417] short [2]: -32768 -> 32767
    [ 2818.303438] unsigned int [4]: 0 -> 4294967295
    [ 2818.303459] int [4]: -2147483648 -> 2147483647
    [ 2818.303480] unsigned long [8]: 0 -> 18446744073709551615
    [ 2818.303501] long [8]: -9223372036854775808 -> 9223372036854775807
    [ 2818.303523] unsigned long long [8]: 0 -> 18446744073709551615
    [ 2818.303544] long long [8]: -9223372036854775808 -> 9223372036854775807
    [ 2818.303566] unsigned int pointer max: 18446744073709551615
```
