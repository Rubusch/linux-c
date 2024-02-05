# Type Sizes

Print the types, sizes and smallest and largest number (in dec).  

## Usage

```
# sudo insmod print_my_types.ko

# dmesg
    ...
    [  423.204262] --- types ---
    [  423.204288] unsigned short [2]: 0 -> 65535
    [  423.204309] short [2]: -32768 -> 32767
    [  423.204320] unsigned int [4]: 0 -> 4294967295
    [  423.204330] int [4]: -2147483648 -> 2147483647
    [  423.204340] unsigned long [8]: 0 -> 18446744073709551615
    [  423.204350] long [8]: -9223372036854775808 -> 9223372036854775807
    [  423.204360] unsigned long long [8]: 0 -> 18446744073709551615
    [  423.204371] long long [8]: -9223372036854775808 -> 9223372036854775807
    [  423.204381] unsigned int pointer max: 18446744073709551615
```