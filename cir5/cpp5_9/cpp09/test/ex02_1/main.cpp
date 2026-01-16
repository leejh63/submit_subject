/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/05 12:53:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2026/01/05 12:53:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cerrno>
#include <climits>

// -------------------- parsing --------------------
// -------------------- parsing / printing --------------------
int p_err(const int num) {
	const char* errp[] = {
		"argc < 2\n",
		"empty argv\n",
		"not num\n",
		"long overfloor\n",
		"negative num\n",
		"INT_MAX num\n",
	};
	
	std::cerr << "[" << num << "]Error: " << errp[num] << std::endl;
	return 1;
}

template <typename Con>
static bool only_p_int(int argc, char* argv[], Con& container)
{
    for (int i = 1; i < argc; ++i) {
        char* end = 0;
        errno = 0;
        long val = std::strtol(argv[i], &end, 10);

        if (*argv[i] == '\0')  return p_err(1);
        if (*end != '\0')      return p_err(2);
        if (errno != 0)        return p_err(3);
        if (val <= 0)          return p_err(4);   // positive only
        if (val > INT_MAX)     return p_err(5);

        container.push_back(static_cast<int>(val));
    }
    return false;
}

static void print_stdout_vec(const char* label, const std::vector<int>& v)
{
    std::cout << label;
    for (size_t i = 0; i < v.size(); ++i) std::cout << v[i] << " ";
    std::cout << "\n";
}

// ==================== Ford–Johnson (vector) ====================
// Node: keep (value, id). id == "index in current recursion's contain"
struct Node {
    int val;
    int id;
    Node(int v=0, int i=0) : val(v), id(i) {}
};

static bool node_less(const Node& a, const Node& b) { return a.val < b.val; }

static size_t find_pos_by_id(const std::vector<Node>& chain, int id)
{
    for (size_t i = 0; i < chain.size(); ++i)
        if (chain[i].id == id) return i;
    return chain.size();
}

static void insert_unbounded(std::vector<Node>& chain, const Node& x)
{
    std::vector<Node>::iterator it =
        std::lower_bound(chain.begin(), chain.end(), x, node_less);
    chain.insert(it, x);
}

static void insert_bounded(std::vector<Node>& chain, const Node& x, const Node& bound_big)
{
    size_t boundPos = find_pos_by_id(chain, bound_big.id);
    std::vector<Node>::iterator first = chain.begin();
    std::vector<Node>::iterator last  = chain.begin() + boundPos;

    std::vector<Node>::iterator it =
        std::lower_bound(first, last, x, node_less);
    chain.insert(it, x);
}

// Jacobsthal order for indices 2..maxIndex (1-based), where b1 is already placed
static std::vector<size_t> build_jacob_order(size_t maxIndex)
{
    std::vector<size_t> order;
    if (maxIndex <= 1) return order;

    // Jacobsthal: J0=0, J1=1, Jn = J(n-1) + 2*J(n-2)
    std::vector<size_t> J;
    J.push_back(0);
    J.push_back(1);
    while (J.back() < maxIndex) {
        size_t n = J.size();
        size_t next = J[n - 1] + 2 * J[n - 2];
        J.push_back(next);
        if (next >= maxIndex) break;
    }

    // blocks: (Jk .. Jk-1+1) descending, skip 1 => lo >= 2
    for (size_t k = 2; k < J.size(); ++k) {
        size_t hi = J[k];
        if (hi > maxIndex) hi = maxIndex;

        size_t lo = J[k - 1] + 1;
        if (lo < 2) lo = 2;

        for (size_t idx = hi; idx >= lo; --idx) {
            order.push_back(idx);
            if (idx == lo) break;
        }
    }

    // safety fill for 2..maxIndex
    std::vector<bool> used(maxIndex + 1, false);
    for (size_t i = 0; i < order.size(); ++i) used[ order[i] ] = true;
    for (size_t i = 2; i <= maxIndex; ++i)
        if (!used[i]) order.push_back(i);

    return order;
}

#ifdef DEBUG_FJ
static void dbg_print_before(const std::vector<Node>& contain)
{
    std::cerr << "비교전 벡터 (idx,val): ";
    for (size_t i = 0; i < contain.size(); ++i)
        std::cerr << "(" << contain[i].id << "," << contain[i].val << ") ";
    std::cerr << "\n";
}

static void dbg_print_big_small(const char* label,
                                const std::vector< std::pair<Node,Node> >& pairs,
                                bool print_big)
{
    std::cerr << label << ": ";
    for (size_t i = 0; i < pairs.size(); ++i) {
        const Node& x = print_big ? pairs[i].first : pairs[i].second;
        std::cerr << "(" << x.id << "," << x.val << ") ";
    }
    std::cerr << "\n";
}

static void dbg_print_leftover(bool has_leftover, const Node& leftover_small)
{
    std::cerr << "비교안된 수: ";
    if (has_leftover) std::cerr << "(" << leftover_small.id << "," << leftover_small.val << ")";
    else std::cerr << "NONE";
    std::cerr << "\n";
}

static void dbg_print_chain_vals(const char* label, const std::vector<Node>& chain)
{
    std::cerr << label << ": ";
    for (size_t i = 0; i < chain.size(); ++i) std::cerr << chain[i].val << " ";
    std::cerr << "\n";
}

static void dbg_print_jacob(const std::vector<size_t>& jacob)
{
    std::cerr << "현재 야콥수(삽입 인덱스, 1-based): ";
    for (size_t i = 0; i < jacob.size(); ++i) std::cerr << jacob[i] << " ";
    std::cerr << "\n";
}

static void dbg_print_small_chain_mapping(const std::vector<Node>& big_chain,
                                          const std::vector<Node>& small_chain)
{
    std::cerr << "pair 매칭(k: big_chain[k] <-> small_chain[k]):\n";
    for (size_t k = 0; k < big_chain.size(); ++k) {
        std::cerr << "  k=" << (k+1)
                  << " big=(idx=" << big_chain[k].id << ",val=" << big_chain[k].val << ")"
                  << " small=(idx=" << small_chain[k].id << ",val=" << small_chain[k].val << ")"
                  << "\n";
    }
}
#endif

static std::vector<Node> FJ_nodes_sort(std::vector<Node>& contain, int depth)
{
    size_t n = contain.size();
    if (n <= 1) return contain;

#ifdef DEBUG_FJ
    std::cerr << "\n===== FJ DEBUG depth=" << depth << " n=" << n << " =====\n";
    dbg_print_before(contain);
#endif

    // 1) 비교쌍 pairs 생성: pairs[i] = (big, small)
    std::vector< std::pair<Node,Node> > pairs;
    pairs.reserve(n / 2);

    std::vector<Node> big_set;
    big_set.reserve(n / 2);

    bool has_leftover = (n % 2 == 1);
    Node leftover_small;
    if (has_leftover) leftover_small = contain[n - 1];

    size_t limit = (n / 2) * 2;
    for (size_t i = 0; i < limit; i += 2) {
        Node x = contain[i];
        Node y = contain[i + 1];
        if (node_less(y, x)) {
            pairs.push_back(std::make_pair(x, y)); // big=x, small=y
            big_set.push_back(x);
        } else {
            pairs.push_back(std::make_pair(y, x)); // big=y, small=x
            big_set.push_back(y);
        }
    }

#ifdef DEBUG_FJ
    dbg_print_big_small("비교후 BIG   (idx,val)", pairs, true);
    dbg_print_big_small("비교후 SMALL (idx,val)", pairs, false);
    dbg_print_leftover(has_leftover, leftover_small);
#endif

    // 2) big 재귀 정렬
    std::vector<Node> big_chain = FJ_nodes_sort(big_set, depth + 1);

#ifdef DEBUG_FJ
    dbg_print_chain_vals("재귀후 벡터(big sorted)", big_chain);
#endif

    size_t p = pairs.size();
    if (p == 0) {
#ifdef DEBUG_FJ
        dbg_print_chain_vals("삽입후 벡터", big_chain);
        std::cerr << "==============================\n";
#endif
        return big_chain;
    }

    // big_chain 순서에 맞춘 small_chain 만들기:
    // small_chain[k] is the small paired with big_chain[k]
    std::vector<Node> small_chain;
    small_chain.reserve(p);

    for (size_t k = 0; k < big_chain.size(); ++k) {
        Node big = big_chain[k];
        for (size_t i = 0; i < pairs.size(); ++i) {
            if (pairs[i].first.id == big.id) {
                small_chain.push_back(pairs[i].second);
                break;
            }
        }
    }

#ifdef DEBUG_FJ
    dbg_print_small_chain_mapping(big_chain, small_chain);
#endif

    // main chain init: [b1] + big_chain
    std::vector<Node> merged_chain;
    merged_chain.reserve(n);
    for (size_t i = 0; i < big_chain.size(); ++i) merged_chain.push_back(big_chain[i]);
    merged_chain.insert(merged_chain.begin(), small_chain[0]); // b1

#ifdef DEBUG_FJ
    dbg_print_chain_vals("메인 벡터(초기: b1 + big_chain)", merged_chain);
#endif

    // Jacob order indices: 2..p (+ p+1 if leftover)
    size_t maxIndex = p + (has_leftover ? 1 : 0);
    std::vector<size_t> jacob = build_jacob_order(maxIndex);

#ifdef DEBUG_FJ
    dbg_print_jacob(jacob);
    std::cerr << "야콥수 삽입 과정:\n";
#endif

    for (size_t t = 0; t < jacob.size(); ++t) {
        size_t k = jacob[t]; // 1-based b_k

        if (k <= p) {
            const Node& big   = big_chain[k - 1];
            const Node& small = small_chain[k - 1];

#ifdef DEBUG_FJ
            std::cerr << "  J=" << k
                      << " -> pair[" << k << "] "
                      << "big=(idx=" << big.id << ",val=" << big.val << ") "
                      << "small=(idx=" << small.id << ",val=" << small.val << ") "
                      << "bound=big\n";
#endif
            insert_bounded(merged_chain, small, big);
        } else {
#ifdef DEBUG_FJ
            std::cerr << "  J=" << k
                      << " -> leftover b" << k
                      << "=(idx=" << leftover_small.id << ",val=" << leftover_small.val << ") "
                      << "bound=NONE(unbounded)\n";
#endif
            insert_unbounded(merged_chain, leftover_small);
        }
    }

#ifdef DEBUG_FJ
    dbg_print_chain_vals("삽입후 벡터", merged_chain);
    std::cerr << "==============================\n";
#endif

    return merged_chain;
}

static std::vector<int> FJ_vec_sort(std::vector<int>& input)
{
    std::vector<Node> nodes;
    nodes.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i)
        nodes.push_back(Node(input[i], (int)i));

    std::vector<Node> sorted_nodes = FJ_nodes_sort(nodes, 0);

    std::vector<int> out;
    out.reserve(sorted_nodes.size());
    for (size_t i = 0; i < sorted_nodes.size(); ++i)
        out.push_back(sorted_nodes[i].val);
    return out;
}

// ==================== main ====================
int main(int argc, char* argv[])
{
    if (argc < 2) return p_err(0);

    std::vector<int> v;
    if (only_p_int(argc, argv, v)) return 1;

    print_stdout_vec("Before: ", v);

    std::vector<int> sorted = FJ_vec_sort(v);

    print_stdout_vec("After:  ", sorted);

    return 0;
}
































