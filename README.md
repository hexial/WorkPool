# WorkPool

I wanted an easy to use implementation of a pool of worker threads that you can submit jobs to,
which will use the C++ std::future 

_There are probably many other implementations, but I thought it was fun to write my own :-)_

```cpp
WorkPool::Pool<int> pool(5);
auto result1 = pool.Execute([&](int a){return 1+a;}, 11);
auto result2 = pool.Execute([&](int a){return 2+a;}, 22);
auto result3 = pool.Execute([&](int a){return 3+a;}, 33);
result1.wait();
spdlog::info("Result1 = {}", result1.get());
result2.wait();
spdlog::info("Result2 = {}", result2.get());
result3.wait();
spdlog::info("Result3 = {}", result3.get());
```

