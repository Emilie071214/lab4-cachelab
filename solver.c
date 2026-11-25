/*
 * solver.c - Cache Lab Bonus: reverse-engineer an unknown cache.
 *
 * The probe strategy:
 *   1. b   - touch offset 0, then the first power-of-two offset that
 *            misses is the block size.
 *   2. P   - P = E * 2^s. Load a growing run of consecutive blocks and
 *            reload it; the first run length that produces any reload
 *            miss is P + 1 (one set has just received E + 1 blocks).
 *   3. s   - For stride 2^t the first evicting run length is
 *            E * 2^(s - min(t, s)) + 1. Testing the candidate value
 *            P / 2^t + 1 distinguishes t <= s from t > s, so a binary
 *            search over t recovers s.
 *   4. E   - P / 2^s.
 *   5. policy - Fill a set, re-touch the oldest block, insert one more.
 *            A hit re-touch makes LRU keep it; FIFO still evicts it;
 *            RANDOM varies from trial to trial.
 *   6. write_allocate - a store miss followed by a load hits only when
 *            the store allocated the block.
 *
 * Only cache_* / flush / report are used, as required.
 */
#include "solver.h"

static int g_b;                 /* block offset bits, found first */

static uint32_t block_addr(unsigned long long idx, int t)
{
    /* Block number idx << t, turned into a byte address. */
    return (uint32_t)((idx << t) << g_b);
}

/*
 * first_miss - Load blocks i * 2^t for i = 0 .. n-1, then reload them in
 * the same order. Returns 1 if any reload misses. If the n blocks all fit
 * in their sets, every reload hits; otherwise an evicted block is missing.
 */
static int first_miss(int n, int t)
{
    int i;

    cache_flush();
    for (i = 0; i < n; i++)
        cache_load(block_addr((unsigned long long)i, t));
    for (i = 0; i < n; i++)
        if (!cache_load(block_addr((unsigned long long)i, t)))
            return 1;
    return 0;
}

/* Smallest n with first_miss(n, t) true. */
static int first_bad_run(int t, int hi)
{
    int lo = 1;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (first_miss(mid, t))
            hi = mid;
        else
            lo = mid + 1;
    }
    return lo;
}

static int find_policy(int s, int e)
{
    int trial, hits = 0;

    if (e == 1)
        return POLICY_LRU;      /* not graded when e == 1 */

    /*
     * 300 independent trials make an accidental uniform outcome for a
     * RANDOM cache (probability at most (1 - 1/e)^300) negligible even
     * for e = 16 (about 4e-9).
     */
    for (trial = 0; trial < 300; trial++) {
        int j;

        cache_flush();
        for (j = 0; j < e; j++)
            cache_load(block_addr((unsigned long long)j, s));
        cache_load(block_addr(0, s));       /* hit: LRU makes it recent */
        cache_load(block_addr((unsigned long long)e, s)); /* eviction */
        if (cache_load(block_addr(0, s)))   /* is block 0 still there? */
            hits++;
    }
    if (hits == 300)
        return POLICY_LRU;
    if (hits == 0)
        return POLICY_FIFO;
    return POLICY_RANDOM;
}

void solve(void)
{
    int b = 0, s, e, p, lo, hi, policy, write_allocate;

    /* ---- b: first power-of-two offset outside the block ---- */
    cache_flush();
    cache_load(0);
    while (b < 31 && cache_load((uint32_t)1 << b))
        b++;
    g_b = b;

    /* ---- P = E * 2^s: first run length that must evict, minus one ---- */
    p = first_bad_run(0, 8193) - 1;

    /* ---- s: largest t such that stride 2^t is still "one set" at P/2^t ---- */
    lo = 1;
    hi = 10;
    while (lo < hi) {
        int mid = lo + (hi - lo + 1) / 2;
        if (first_miss((p >> mid) + 1, mid))
            lo = mid;
        else
            hi = mid - 1;
    }
    s = lo;
    e = p >> s;

    /* ---- policy ---- */
    policy = find_policy(s, e);

    /* ---- write_allocate: store miss, then load the same block ---- */
    cache_flush();
    cache_store(block_addr(0, 0));
    write_allocate = cache_load(block_addr(0, 0)) ? 1 : 0;

    report(b, s, e, policy, write_allocate);
}
