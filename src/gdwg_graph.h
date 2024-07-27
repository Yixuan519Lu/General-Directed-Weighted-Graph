#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
namespace std {
	template<typename T1, typename T2>
	struct hash<std::pair<T1, T2>> {
		auto operator()(const std::pair<T1, T2>& p) const -> std::size_t {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return (h1 << 1) ^ h2;
		}
	};
} // namespace std
namespace gdwg {
	template<typename N, typename E>
	class graph;
	template<typename N, typename E>
	class edge {
	 public:
		virtual ~edge() noexcept = default;
		virtual auto print_edge() const -> std::string = 0;
		virtual auto is_weighted() const noexcept -> bool = 0;
		virtual auto get_weight() const noexcept -> std::optional<E> = 0;
		virtual auto get_nodes() const noexcept -> std::pair<N, N> = 0;
		virtual auto operator==(const edge<N, E>& rhs) const noexcept -> bool = 0;

	 private:
		friend class graph<N, E>;
	};
	template<typename N, typename E>
	class weighted_edge : public edge<N, E> {
	 public:
		weighted_edge(const N& src, const N& dst, const E& weight) noexcept
		: src_{src}
		, dst_{dst}
		, weight_{weight} {}
		auto print_edge() const -> std::string override {
			auto oss = std::ostringstream{};
			oss << src_ << " -> " << dst_ << " | W | " << weight_;
			return oss.str();
		}
		auto is_weighted() const noexcept -> bool override {
			return true;
		}
		auto get_weight() const noexcept -> std::optional<E> override {
			return weight_;
		}
		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {src_, dst_};
		}
		auto operator==(const edge<N, E>& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<const weighted_edge<N, E>*>(&rhs)) {
				return src_ == obj->src_ and dst_ == obj->dst_ and weight_ == obj->weight_;
			}
			return false;
		}

	 private:
		N src_;
		N dst_;
		E weight_;
		friend class graph<N, E>;
	};
	template<typename N, typename E>
	class unweighted_edge : public edge<N, E> {
	 public:
		unweighted_edge(const N& src, const N& dst) noexcept
		: src_{src}
		, dst_{dst} {}
		auto print_edge() const -> std::string override {
			auto oss = std::ostringstream{};
			oss << src_ << " -> " << dst_ << " | U";
			return oss.str();
		}
		auto is_weighted() const noexcept -> bool override {
			return false;
		}
		auto get_weight() const noexcept -> std::optional<E> override {
			return std::nullopt;
		}
		auto get_nodes() const noexcept -> std::pair<N, N> override {
			return {src_, dst_};
		}
		auto operator==(const edge<N, E>& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<const unweighted_edge<N, E>*>(&rhs)) {
				return src_ == obj->src_ and dst_ == obj->dst_;
			}
			return false;
		}

	 private:
		N src_;
		N dst_;
		friend class graph<N, E>;
	};
	template<typename N, typename E>
	class graph {
	 public:
		graph() = default;
		graph(graph&& other) noexcept {
			nodes_ = std::move(other.nodes_);
			edges_ = std::move(other.edges_);
			other.clear();
		}
		graph(const graph& other) {
			nodes_ = other.nodes_;
			edges_ = other.edges_;
		}
		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto ite = first; ite != last; ++ite) {
				nodes_.emplace(*ite);
			}
		}
		auto operator=(graph&& other) noexcept -> graph& {
			if (this != &other) {
				nodes_ = std::move(other.nodes_);
				edges_ = std::move(other.edges_);
				other.clear();
			}
			return *this;
		}
		auto operator=(const graph& other) -> graph& {
			if (this != &other) {
				nodes_ = other.nodes_;
				edges_ = other.edges_;
			}
			return *this;
		}
		void clear() noexcept {
			nodes_.clear();
			edges_.clear();
		}

		auto insert_node(const N& value) -> bool {
			if (is_node(value)) {
				throw std::runtime_error("Duplicate node");
			}
			return nodes_.emplace(value).second;
		}
		auto insert_edge(const N& src, const N& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge when either src or dst node does "
				                         "not exist");
			}
			auto& edge_set = edges_[src];
			for (const auto& edge : edge_set) {
				if (edge.first == dst and edge.second == weight) {
					return false;
				}
			}
			return edge_set.emplace(dst, weight).second;
		}
		[[nodiscard]] auto is_node(const N& value) const noexcept -> bool {
			return nodes_.find(value) != nodes_.end();
		}
		auto empty() const noexcept -> bool {
			return nodes_.empty() and edges_.empty();
		}
		auto erase_node(const N& value) -> bool {
			if (not is_node(value)) {
				return false;
			}
			nodes_.erase(value);
			edges_.erase(value);
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (it->first == value) {
						it = dst_set.erase(it);
					}
					else {
						++it;
					}
				}
			}
			return true;
		}
		auto erase_edge(const N& src, const N& dst, std::optional<E> weight = std::nullopt) -> bool {
			if (not is_node(src) or not is_node(dst)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge on src or dst if they don't exist "
				                         "in the graph");
			}
			const auto src_it = edges_.find(src);
			if (src_it == edges_.end()) {
				return false;
			}
			auto& dst_set = src_it->second;
			auto erased = false;
			if (weight) {
				const auto edge_it = dst_set.find({dst, *weight});
				if (edge_it != dst_set.end()) {
					dst_set.erase(edge_it);
					erased = true;
				}
			}
			else {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (it->first == dst) {
						it = dst_set.erase(it);
						erased = true;
					}
					else {
						++it;
					}
				}
			}
			if (dst_set.empty()) {
				edges_.erase(src);
			}
			return erased;
		}
		auto replace_node(const N& old_data, const N& new_data) -> bool {
			if (not is_node(old_data)) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}
			if (is_node(new_data)) {
				return false;
			}
			nodes_.emplace(new_data);
			if (auto it = edges_.find(old_data); it != edges_.end()) {
				for (const auto& edge : it->second) {
					edges_[new_data].emplace(edge);
				}
				edges_.erase(it);
			}
			for (auto& [src, dst_set] : edges_) {
				for (auto it = dst_set.begin(); it != dst_set.end();) {
					if (it->first == old_data) {
						dst_set.emplace(new_data, it->second);
						it = dst_set.erase(it);
					}
					else {
						++it;
					}
				}
			}
			nodes_.erase(old_data);
			return true;
		}
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			for (const auto& node : g.nodes_) {
				os << node << " (\n";
				if (auto it = g.edges_.find(node); it != g.edges_.end()) {
					for (const auto& edge_pair : it->second) {
						if (edge_pair.second == std::nullopt) {
							os << "  " << node << " -> " << edge_pair.first << " | U\n";
						}
					}
					for (const auto& edge_pair : it->second) {
						if (edge_pair.second != std::nullopt) {
							os << "  " << node << " -> " << edge_pair.first << " | W | " << *edge_pair.second << "\n";
						}
					}
				}
				os << ")\n";
			}
			return os;
		}

	 private:
		std::set<N> nodes_;
		std::map<N, std::set<std::pair<N, std::optional<E>>>> edges_;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H