valgrind --suppressions=readline.supp --leak-check=full --show-leak-kinds=all ./minishell
valgrind --suppressions=libreadline.supp --leak-check=full --show-leak-kinds=all ./minishell


해야할 것: $?, exit, signal
suggestion 1: $? << 이거 env의 맨 앞에 저장하는 거 어떤지... export나 env할 때 첫 env 다음꺼부터 하면 되고, get_env를 쓰기에 편하다. 어차피 숫자인 주제에 ft_atoi랑 ft_itoa를 써서 저장해야 한다는 단점이 있다. 그리고 어차피 255이하니까 malloc(4)한 번만 하고, 재할당을 하지 않아도 된다.

exit: free(exp(전체)), free(t_env) exit하면 되나?

args[0]이 if (builtin)/ else if (절대경로) / else if (상대경로 )/ else if (PATH) / else ( == command not found)를 찾고 나서 fork를 해야 할 것 같습니다.
지금은 command not found도 built in 아니라고 fork했더니 execve실패 이후 leak나서 죽습니다.
