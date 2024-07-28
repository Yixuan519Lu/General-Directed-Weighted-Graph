#include "gdwg_graph.h"

#include <catch2/catch.hpp>

TEST_CASE("gdwg::graph") {
	SECTION("Constructors") {
		SECTION("Default constructor") {
			auto g1 = gdwg::graph<int, std::string>{};
			CHECK(g1.empty());
			auto g2 = gdwg::graph<double, double>{};
			CHECK(g2.empty());
		}
		SECTION("Move constructor") {
			auto g1 = gdwg::graph<int, std::string>{1, 2, 3};
			auto n = 5;
			g1.insert_node(n);
			auto g2 = std::move(g1);
			CHECK(not g1.is_node(1));
			CHECK(not g1.is_node(2));
			CHECK(not g1.is_node(3));
			CHECK(not g1.is_node(5));
			CHECK(g1.empty());
			CHECK(g2.is_node(1));
			CHECK(g2.is_node(2));
			CHECK(g2.is_node(3));
			CHECK(g2.is_node(5));
		}
	}
	SECTION("Modifiers") {
		SECTION("insert_node") {
			auto g = gdwg::graph<int, std::string>{};

			SECTION("Insert a new node") {
				CHECK(g.insert_node(1));
				CHECK(g.is_node(1));
			}

			SECTION("Insert a duplicate node") {
				g.insert_node(2);
				CHECK(not g.insert_node(2));
			}

			SECTION("Insert multiple nodes") {
				CHECK(g.insert_node(3));
				CHECK(g.insert_node(4));
				CHECK(g.is_node(3));
				CHECK(g.is_node(4));
			}
		}
		SECTION("replace_node") {
			auto g = gdwg::graph<int, int>{1, 2, 3};
			g.insert_edge(1, 2, 1);
			g.insert_edge(2, 3, 2);
			SECTION("Replacing success") {
				CHECK(g.replace_node(1, 5));
				auto out = std::ostringstream{};
				out << g;
				auto const expected_output = std::string_view(R"(
2 (
  2 -> 3 | W | 2
)
3 (
)
5 (
  5 -> 2 | W | 1
)
)");
				CHECK(out.str() == expected_output);
			}
			SECTION("Replacing node not exists") {
				CHECK_THROWS_WITH(g.replace_node(4, 5),
				                  "Cannot call gdwg::graph<N, E>::replace_node on a node that doesn't exist");
			}
			SECTION("Replacing node exists") {
				CHECK(not g.replace_node(1, 2));
				auto out = std::ostringstream{};
				out << g;
				auto const expected_output = std::string_view(R"(
1 (
  1 -> 2 | W | 1
)
2 (
  2 -> 3 | W | 2
)
3 (
)
)");
				CHECK(out.str() == expected_output);
			}
		}
		SECTION("insert_node") {
			SECTION("Merging success") {
				auto g = gdwg::graph<int, int>{1, 2, 3};
				g.insert_edge(1, 2, 1);
				g.insert_edge(1, 2);
				g.insert_edge(1, 1, 3);
				g.insert_edge(1, 3, 2);
				g.insert_edge(2, 3, 3);
				g.insert_edge(3, 2);
				g.insert_edge(3, 1, 4);
				g.insert_edge(3, 2, 5);

				g.merge_replace_node(1, 3);
				auto out = std::ostringstream{};
				out << g;
				auto const expected_output = std::string_view(R"(
2 (
  2 -> 3 | W | 3
)
3 (
  3 -> 2 | U
  3 -> 2 | W | 1
  3 -> 2 | W | 5
  3 -> 3 | W | 2
  3 -> 3 | W | 3
  3 -> 3 | W | 4
)
)");
				CHECK(out.str() == expected_output);
			}
			SECTION("Merging success 2") {
				auto g = gdwg::graph<char, int>{'A', 'B', 'C', 'D'};
				g.insert_edge('A', 'B', 3);
				g.insert_edge('C', 'B', 2);
				g.insert_edge('D', 'B', 4);

				g.merge_replace_node('B', 'A');
				auto out = std::ostringstream{};
				out << g;
				auto const expected_output = std::string_view(R"(
A (
  A -> A | W | 3
)
C (
  C -> A | W | 2
)
D (
  D -> A | W | 4
)
)");
				CHECK(out.str() == expected_output);
			}
		}
	}
	SECTION("graph edges()") {
		using graph = gdwg::graph<int, int>;

		SECTION("Edges retrieval") {
			auto g = graph{};
			g.insert_node(1);
			g.insert_node(2);
			g.insert_node(3);
			g.insert_edge(1, 2, 5);
			g.insert_edge(1, 2, 10);
			g.insert_edge(1, 2);
			g.insert_edge(1, 3, 15);

			SECTION("Valid edges retrieval") {
				auto edges_1_2 = g.edges(1, 2);
				CHECK(edges_1_2.size() == 3);
				CHECK(edges_1_2[0]->get_weight() == std::nullopt);
				CHECK(edges_1_2[1]->get_weight() == 5);
				CHECK(edges_1_2[2]->get_weight() == 10);

				auto edges_1_3 = g.edges(1, 3);
				CHECK(edges_1_3.size() == 1);
				CHECK(edges_1_3[0]->get_weight() == 15);
			}
			SECTION("Edges not exists") {
				auto edges_2_3 = g.edges(2, 3);
				CHECK(edges_2_3.empty());
			}
			SECTION("Nodes not exists") {
				CHECK_THROWS_WITH(g.edges(4, 2),
				                  "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the graph");
				CHECK_THROWS_WITH(g.edges(1, 4),
				                  "Cannot call gdwg::graph<N, E>::edges if src or dst node don't exist in the graph");
			}
		}
	}
	SECTION("Extractor") {
		SECTION("Example test") {
			using graph = gdwg::graph<int, int>;
			auto const v = std::vector<std::tuple<int, int, std::optional<int>>>{
			    {4, 1, -4},
			    {3, 2, 2},
			    {2, 4, std::nullopt},
			    {2, 1, 1},
			    {6, 2, 5},
			    {6, 3, 10},
			    {1, 5, -1},
			    {3, 6, -8},
			    {4, 5, 3},
			    {5, 2, std::nullopt},
			};

			auto g = graph{};
			for (const auto& [from, to, weight] : v) {
				g.insert_node(from);
				g.insert_node(to);
				if (weight.has_value()) {
					g.insert_edge(from, to, weight.value());
				}
				else {
					g.insert_edge(from, to);
				}
			}
			g.insert_node(64);
			auto out = std::ostringstream{};
			out << g;
			auto const expected_output = std::string_view(R"(
1 (
  1 -> 5 | W | -1
)
2 (
  2 -> 4 | U
  2 -> 1 | W | 1
)
3 (
  3 -> 2 | W | 2
  3 -> 6 | W | -8
)
4 (
  4 -> 1 | W | -4
  4 -> 5 | W | 3
)
5 (
  5 -> 2 | U
)
6 (
  6 -> 2 | W | 5
  6 -> 3 | W | 10
)
64 (
)
)");
			CHECK(out.str() == expected_output);
		}
		SECTION("Empty graph test") {
			using graph = gdwg::graph<int, std::string>;
			auto g = graph{};
			auto out = std::ostringstream{};
			out << g;
			CHECK(out.str() == "");
		}
	}
}