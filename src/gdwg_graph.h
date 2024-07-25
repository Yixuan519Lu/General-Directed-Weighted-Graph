#ifndef GDWG_GRAPH_H
#define GDWG_GRAPH_H
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// TODO: Make both graph and edge generic
//       ... this won't just compile
//       straight away
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
		virtual auto operator==(edge const& rhs) const noexcept -> bool = 0;

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
		auto operator==(edge<N, E> const& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<weighted_edge<N, E> const*>(&rhs)) {
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
		auto operator==(edge<N, E> const& rhs) const noexcept -> bool override {
			if (auto* obj = dynamic_cast<unweighted_edge<N, E> const*>(&rhs)) {
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
		~graph() = default;
		graph(graph&& input_graph) noexcept {
			nodes_ = std::move(input_graph.nodes_);
			edges_ = std::move(input_graph.edges_);
		}
		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()) {}
		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			for (auto ite = first; ite != last; ++ite) {
				nodes_.emplace_back(*ite);
			}
		}

	 private:
		std::vector<N> nodes_;
		std::vector<std::unique_ptr<edge<N, E>>> edges_;
	};
} // namespace gdwg

#endif // GDWG_GRAPH_H
