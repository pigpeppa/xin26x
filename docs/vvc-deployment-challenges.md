[Top level](../README.md)

# VVC Deployment Challenges

## Introduction

It has been nearly five years since the release of the Versatile Video Coding (VVC) standard, yet its overall adoption remains limited. Although some vendors and platforms have started to experiment with VVC deployment, its industry adoption has been noticeably slower compared with the early adoption of High Efficiency Video Coding (HEVC).

When analyzing this situation, factors such as the hardware ecosystem, patent licensing models, and the structure of industry alliances are often mentioned. However, in this discussion we temporarily set aside these external factors and focus only on the standard design itself and commercial software implementation, in order to explore some technical reasons that may have influenced the practical adoption of VVC.

From an implementation perspective, VVC does achieve improvements in compression efficiency. At the same time, however, it introduces a large number of new coding tools, which significantly increase algorithmic complexity, the difficulty of encoder implementation, and the cost of engineering optimization. For many real-world systems, these factors may partially offset the benefits brought by the improved compression efficiency.

The following sections discuss this issue from several perspectives. These factors together increase the difficulty of deploying VVC in practical engineering systems and may also be one of the important reasons why the industry remains cautious about adopting VVC at large scale.

## Limited Bitrate Reduction Ceiling

Compared with previous video coding standards, VVC appears to exhibit a certain “ceiling effect” in terms of bitrate reduction. From AVC to HEVC, the standard achieved roughly a 50% reduction in bitrate. In contrast, the overall bitrate reduction of VVC relative to HEVC is less than 40%. To some extent, this result falls short of the industry's expectations for a new generation of video coding standards. 

In my view, this may be partly related to the time pressure during the standardization process. After the emergence of competing standards such as AOMedia Video 1 (AV1), the release timeline of VVC may have been somewhat accelerated. From the final outcome, both the BD-rate improvement and the increase in computational complexity suggest that VVC represents a compromise under a fixed timeline. 

## Declining Coding Gain per Computation Cost

Over the past two decades of video coding standardization, the computational cost required to achieve the same level of PSNR improvement has been steadily increasing. In other words, the efficiency ratio of PSNR per unit of computation cost has been continuously declining. This trend becomes even more pronounced in VVC. Compared with AVC, HEVC reduces the bitrate by approximately 50%, while increasing the computational complexity by about four times. In contrast, VVC further reduces the bitrate by about 40% relative to HEVC, but the computational complexity increases by nearly ten times.

This indicates that in newer generations of video coding standards, the cost-effectiveness of new coding tools—measured by compression gain relative to computational cost—is gradually declining. Compared with HEVC, VVC introduces a large number of additional coding tools. Although these tools further exploit compression potential, they often require significantly higher computational cost to achieve relatively limited coding gains.

From an engineering perspective, this continuously decreasing trend of coding gain per computation cost implies that the future development of video coding technologies should place greater emphasis on the overall balance between compression efficiency and computational complexity, rather than solely pursuing lower BD-rate. Under such circumstances, a more reasonable direction for future development may not be to blindly pursue higher compression efficiency, but rather to improve the efficiency ratio of BD-rate reduction per computation cost. In other words, the goal should be to achieve a better engineering trade-off between compression gain and computational cost.

## Distributed Coding Gain across Coding Tools

In previous video coding standards, improvements in overall PSNR were often mainly driven by a small number of key coding tools. The introduction of some core tools could typically deliver several percentage points or more of BD-rate reduction, thereby becoming the primary contributors to compression efficiency gains. As a result, during encoder implementation, engineering optimization efforts were often concentrated on these critical modules.

In Versatile Video Coding (VVC), however, the improvement in compression efficiency exhibits a more distributed characteristic. Many newly introduced coding tools individually provide less than 0.5% BD-rate reduction, but through the accumulation of a large number of such tools, a noticeable overall gain in compression efficiency can still be achieved. This “fragmented” gain structure means that each tool brings only limited benefit on its own, yet the combined effect can still produce meaningful coding gains. 

This trend becomes even more evident in the research of the Enhanced Compression Model (ECM). An increasing number of small incremental improvements are introduced into the coding framework, each contributing relatively limited gains but collectively forming a large set of tools. While this distributed coding gain pattern can theoretically continue to explore additional compression potential, it also introduces new challenges in engineering practice. The encoder architecture becomes more complex; the interactions among different tools become harder to manage; and for commercial encoder implementations, the costs of development, optimization, and maintenance increase significantly. 

Therefore, in future video coding research, how to achieve a better balance between compression efficiency improvement and implementation complexity control may become an increasingly important issue.

## Increasing Difficulty of SIMD Optimization

Compared with the largely regularized computations in AVC and HEVC, many coding tools introduced in the newer VVC standard exhibit more complex algorithmic structures and stronger data dependencies, which significantly increase the difficulty of SIMD optimization. For example, tools such as Dependent Quantization (DQ) and Decoder-side Motion Vector Refinement (DMVR), which provide notable PSNR improvements, involve strong data dependencies or irregular memory access patterns, making efficient vectorization difficult.

In addition, the modifications to the Deblocking Filter in VVC introduce more conditional branches and neighborhood dependencies, further complicating the implementation of efficient SIMD optimizations. 

As a result, compared with AVC and HEVC, VVC poses greater challenges in achieving high-performance SIMD implementations.

## Uncertain Efficiency Advantages under Fast Encoding Configurations

This point is often overlooked. It is undeniable that under low-speed encoding configurations, VVC can achieve a higher PSNR per computation cost compared with HEVC. However, in practical applications where fast encoding modes are more relevant, whether VVC can still maintain a similar level of computational efficiency as HEVC remains an open question worth discussing.

In real-world system deployments, when considering migrating an encoding system from HEVC to VVC, it is generally expected that such an upgrade should not introduce noticeable negative impacts. However, according to our experimental results, in high-speed encoding scenarios, VVC may require approximately 20% additional computation to achieve the same PSNR as HEVC.

The above observations are based on experiments conducted using the Xin26x encoder implementation. During the evaluation, we restricted the configuration to use only the coding tools already supported in HEVC, while disabling the newly introduced tools in VVC. In addition, we attempted to reuse common code paths between the HEVC and VVC implementations as much as possible. Under these conditions, the observed differences mainly originate from Intra Prediction, Deblocking, Entropy Coding/Estimation, and several minor modifications related to Inter prediction. Later, we will further discuss the testing methodology, as well as optimization techniques that help VVC approach the efficiency of HEVC under fast encoding configurations.

## Challenges from Slowing Processor Performance Growth

Looking back at the early days of High Efficiency Video Coding, there was also a noticeable gap between encoder computational complexity and hardware processing capability. However, this gap was quickly narrowed as processor performance improved rapidly and encoder algorithms continued to be optimized. It is worth noting that the release of HEVC coincided with the emergence of AVX2 in mainstream processors. In practice, AVX2 provided a critical foundation for large-scale SIMD optimizations in HEVC encoders and played an important role in facilitating the engineering implementation and industry adoption of HEVC.

In contrast, during the era of Versatile Video Coding, the pace of hardware development has not continued the same growth trajectory. Although AVX-512 theoretically offers higher levels of parallel computing capability, its adoption in mainstream processors has remained limited, and the practical performance gains have fallen short of early expectations. Meanwhile, the growth of general-purpose computing capability in x86 processors has slowed in recent years. As a result, the substantial increase in computational complexity introduced by VVC compared with HEVC can no longer be absorbed solely through the natural evolution of processor performance, which to some extent affects the engineering deployment and practical adoption of VVC encoders.

Historical experience suggests that the successful engineering deployment of video coding standards often depends on the co-evolution of algorithmic complexity and hardware computing capability. In the VVC era, however, this synergy appears to be weaker than in previous generations.

## Conclusion and Outlook

In summary, Versatile Video Coding continues the long-standing trend of steady progress in video compression efficiency. At the same time, however, it also exposes several new challenges. These challenges do not arise solely from the standard design itself, but gradually emerge from the balance among standard capabilities, engineering implementation, and hardware computing capacity.

Historical experience shows that advances in video coding technology have never been driven by standards alone. A standard typically defines the theoretical compression potential, while translating that potential into practical system capability requires extensive engineering implementation, algorithm optimization, and continuous innovation in system and architectural design. The evolution of hardware processor performance also plays a crucial role in this process. As processor computing power and parallelism increase, complex algorithms in video encoders can gradually be absorbed through engineering optimization, thereby facilitating the practical deployment of new coding technologies.

However, at the stage where VVC is being introduced, this synergy appears to be more constrained than in previous generations. As a result, future progress in video coding technology will require efforts from multiple directions. On the one hand, standard designers need to pay greater attention to the balance between compression efficiency and computational complexity when introducing new coding tools. On the other hand, encoder developers must continue to explore new optimization strategies and architectural innovations at the implementation level. At the same time, effectively leveraging modern processor architectures and parallel computing capabilities will remain an important challenge for practical implementations.

Only when standard design, engineering implementation, and hardware capability evolve in a mutually reinforcing manner can new coding technologies achieve both theoretical advancement and efficient deployment in real-world systems. From this perspective, the coordinated progress of standard evolution, engineering practice, and hardware development may well be the key driving force behind the next stage of video coding technology.

