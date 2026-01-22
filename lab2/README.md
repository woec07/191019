# ATTENTION - NOT UP TO DATE FOR LAB EXAM 2 -> READ HOWTO.MD


# 2.0 Definitions

## Module Hierarchy
* **Latency:** the total time it takes for the function to finish one execution from start to finish.
* **Interval:** the time you must wait before you can call the function again (if not pipelined yet: latency = interval; if pipelined interval == 1).
* **LUT (Look-Up Table):** The fundamental programmable logic gate used for boolean operations and small distributed memory (LUTRAM).
* **FF (Flip-Flop):** A 1-bit storage register used to hold state and synchronize data between pipeline stages.
* **DSP (Digital Signal Processor):** A hardened silicon block optimized for high-speed, efficient arithmetic (multiplication/addition) without using general logic.

## Performance Profile
*The following metrices describe the behavior of a specific loop (inside our function)*
* **Iteration Latency:** the time it takes to complete one single iteration of the loop.
* **Initiation Interval:** the time between two loop iterations (if sequential: = iteration latency, if pipelined = 1).
* **Trip Count:** the # of times the loop is executed.

## Total time
* **Sequential:** `latency = trip count * iteration latency`
* **Pipelined:** `latency = (trip count * Initiation interval) + iteration latency`
    * *(first term: launch phase, i. e. start all iterations, second term: wait till last iteration has finished (one cycle))*
---
# 2.1 Questions

**1. What is C-code validation and C-code synthesis in Vivado-HLS?**
* **C-code validation:** ensuring the logical correctness of the implemented C-code
* **C-code synthesis:** creating a HW component (Register Transfer Level, short: RTL) from the C-code

**2. What is Local vs Global optimization?**
* **Local optimization:** optimizing within a specific function or a loop body (loop pipelining (i. e. using `[PIPELINE]` within a certain loop), loop unrolling)
* **Global optimization:** parallelism between multiple function calls of the method (pipelining of the function itself using (`[DATAFLOW]`))

**3. What are Directives in Vivado-HLS? List the available directives in Vivado HLS and provide a one-line description for each.**
used in lab-module 2:
* `[PIPELINE]`: pipelining a loop, so that multiple iterations of a loop run in a pipeline (automatically also `[LOOP_FLATTEN]` for perfect nested loops and loop unrolling of the inner loop if `[PIPELINE]` is applied for outer loops)
* `[ARRAY_PARTITION]`: divides the whole BRAM into multiple smaller memories, each equipped with 2 I/O so that I/O is no bottleneck anymore
* `[DATAFLOW]`: pipelines the overall functions, i.e. sub-functions of the function are pipelined (parallel executed), only has impact if input-stream of function is applied
* `[INLINE]`: flattens sequential loop hierarchies, merges its logic direct into the calling function (boundary of a sub-function is dissolved)
* TODO: other directives

**4. How does loop-unrolling works in optimizing the design?**
loop-unrolling replicates the HW blocks, so parallel execution on HW level can be achieved -> higher area usage, but faster

**5. What is the difference between `[DATAFLOW]` and function-level PIPELINE directives?**
`[DATAFLOW]` does pipelining on a global task-level (hence the execution of its sub-functions is parallelized), where `[PIPELINE]` does pipelining on a local loop level (inside a function)

**6. What is the difference between `[LOOP FLATTEN]` and `[UNROLL]` directive?**
`[LOOP FLATTEN]` merges two perfectly nested loops into one loop, in order to remove the control overhead when traversing through the two different loops
`[UNROLL]` replicates the HW blocks, so parallel execution on HW level can be achieved

**7. What is the difference between loop unrolling and pipelining?**
loop unrolling (spatial parallelism): physically replicates the hardware logic (e.g., builds 4 adders instead of 1) to execute multiple loop iterations simultaneously. It increases area significantly.
pipelining (temporal parallelism): Inserts registers between logic stages to keep the existing hardware busy. it starts the next iteration before the current one finishes, increasing throughput with minimal area increase.

**8. What were the indicators that led to the conclusion that the initial design was not pipelined in Step 3 of the DCT-HLS tutorial?**
latency = interval, which meant that we need to wait a whole program execution before the next execution is possible

**9. Why didn’t we apply `[PIPELINE]` -directive directly to the dct_1d in Step 4 of DCT-HLS tutorial?**
- area explosion: `[PIPELINE]` -directive automatically unrolls all loops inside, so this would cost much area
- parent dependency: dct_1d is called sequentially by the loops in dct_2d: just making dct_1d faster would not fix the sequential parent loop, which is still slow then

**10. What were the indicators that led to the conclusion that an imperfect loop nest was blocking the loop-pipelining in Step 5 of the DCT-HLS tutorial?**
the logic between the loops was the indicator that there is an imperfect loop nest -> warning after synthesis, also there was still a loop-hierarchy -> so an imperfect loop nest was blocking

**11. What were the indicators that led to the conclusion that data-dependency was ”bottlenecking” the design in Step 6 of the DCT-HLS tutorial?**
- high utilization for the specific block RAM (active in each cycle, see Resource Profile)
- warning in console
- drift in performance viewer during loop iteration

**12. Describe briefly how an HDL design from Vivado-HLS can be exported to Vivado’s IP-Catalog.**
Solution -> Export RTL
