---
layout: post
title: "Part 1: Building a not so fast, not lock-free, yet concurrent queue"
---

Starting off with the basics, there are a few key components required to create a concurrent queue that will help us build a foundation for later improvements. The name itself already gives us a great hint at what we’re trying to achieve:

- Speed
- Lock-free
- Concurrent
- Queue

Even though we’re not tackling all of these aspects just yet, we can still set up a framework that allows us to prove that what we’re building meets each requirement as we go. We’ll do this using GTest, where we’ll create multiple tests to assert different use cases. For example, we’ll want to verify that:
- Our queue behaves like a proper queue, meaning items come out in the same order they were inserted, regardless of the number of producers or consumers.
- We allow concurrency: multiple producers and consumers can interact with the queue without race conditions or data duplication.

These are just the correctness basics of a concurrent queue. But correctness alone doesn’t make it fast.

Speed and being lock-free go hand in hand. When a concurrent queue relies on locks, it loses a lot of potential performance. Every thread has to wait for others to finish before proceeding. A natural first optimization idea would be to separate reader and writer locks, since producers and consumers operate on opposite ends of the queue and shouldn’t block each other unnecessarily.

Later on, we’ll explore other mechanisms to improve speed, such as ring buffers, atomic variables, and other lock-free techniques. In all these cases, we’ll want a way to measure our progress. Something that shows a provable performance gain in exchange for our efforts. That’s where our benchmarking framework comes in. It will measure how long our queue takes to handle specific workloads under varying levels of concurrency with different numbers of producers and consumers.

So, let’s dive in.

## 
