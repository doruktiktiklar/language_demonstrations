/*
//meson.build
project('conc_func_res_collect', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20', '-fsanitize=address'])

executable('conc_func_res_collect',
           'conc_func_res_collect.cpp',
           install : true)

*/
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>

int task00(bool a0) { return 33; }

std::string task01(int a0) { return "fdsf"; }

class T00 {
public:
  bool mem_b;
  std::string mem_s;
};

T00 task02(T00 a0) { return T00{.mem_b = true, .mem_s = "dfs"}; }

using OUT_TYPES = std::variant<int, std::string, T00>;
using KEKW = std::pair<std::mutex, std::vector<OUT_TYPES>>;
using IN_TYPES = std::variant<bool, int, T00>;
using FUNC_TYPES =
    std::variant<std::function<int(bool)>, std::function<std::string(int)>,
                 std::function<T00(T00)>>;

template <typename T>
std::function<OUT_TYPES(T)> func_out_up_cast(std::function<int(T)> f) {
  return [f](T in_arg) {
    OUT_TYPES outs = f(in_arg);
    return outs;
  };
}

template <typename T>
std::function<OUT_TYPES(T)> func_out_up_cast(std::function<std::string(T)> f) {
  return [f](T in_arg) {
    OUT_TYPES outs = f(in_arg);
    return outs;
  };
}

template <typename T>
std::function<OUT_TYPES(T)> func_out_up_cast(std::function<T00(T)> f) {
  return [f](T in_arg) {
    OUT_TYPES outs = f(in_arg);
    return outs;
  };
}

template <typename T>
void collector_wrapper(std::function<OUT_TYPES(T)> task, T in_arg,
                       KEKW &result_list) {
  OUT_TYPES result = task(in_arg);
  auto lg = std::lock_guard(result_list.first);
  result_list.second.push_back(result);
}

template <typename T>
void dispatcher(FUNC_TYPES task, T in_arg, KEKW &result_list) {
  if (std::holds_alternative<std::function<int(bool)>>(task) &&
      std::holds_alternative<bool>(in_arg)) {
    std::function<int(bool)> curr_task =
        std::get<std::function<int(bool)>>(task);
    bool curr_input = std::get<bool>(in_arg);
    auto f0 = func_out_up_cast(curr_task);
    std::thread curr_thread(collector_wrapper<bool>, f0, curr_input,
                            std::ref(result_list));
    curr_thread.join();
  } else if (std::holds_alternative<std::function<std::string(int)>>(task) &&
             std::holds_alternative<int>(in_arg)) {
    std::function<std::string(int)> curr_task =
        std::get<std::function<std::string(int)>>(task);
    int curr_input = std::get<int>(in_arg);
    auto f0 = func_out_up_cast(curr_task);
    std::thread curr_thread(collector_wrapper<int>, f0, curr_input,
                            std::ref(result_list));
    curr_thread.join();
  } else if (std::holds_alternative<std::function<T00(T00)>>(task) &&
             std::holds_alternative<T00>(in_arg)) {
    std::function<T00(T00)> curr_task = std::get<std::function<T00(T00)>>(task);
    T00 curr_input = std::get<T00>(in_arg);
    auto f0 = func_out_up_cast(curr_task);
    std::thread curr_thread(collector_wrapper<T00>, f0, curr_input,
                            std::ref(result_list));
    curr_thread.join();
  }
}

int main() {
  KEKW results_list;
  dispatcher(task00, std::variant<bool, int, T00>(true), results_list);
  dispatcher(task01, std::variant<bool, int, T00>(2), results_list);
  dispatcher(task02,
             std::variant<bool, int, T00>(T00{.mem_b = false, .mem_s = "dfds"}),
             results_list);
  for (auto a : results_list.second) {
    if (std::holds_alternative<int>(a)) {
      std::cout << "int value: " << std::get<int>(a) << "\n";
    } else if (std::holds_alternative<std::string>(a)) {
      std::cout << "string value: " << std::get<std::string>(a) << "\n";
    } else if (std::holds_alternative<T00>(a)) {
      T00 t00 = std::get<T00>(a);
      std::cout << "T00 value: mem_b: " << t00.mem_b << " mem_s: " << t00.mem_s
                << "\n";
    }
  }
}

