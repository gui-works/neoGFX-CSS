﻿/*
neogfx C++ App/Game Engine - Examples - Games - Chess
Copyright(C) 2020 Leigh Johnston

This program is free software: you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <chess/ai_thread.hpp>
#include <chess/mailbox.hpp>
#include <chess/bitboard.hpp>

namespace chess
{
    template <typename Representation>
    inline basic_position<Representation>& eval_board()
    {
        thread_local basic_position<Representation> sEvalBoard = {};
        return sEvalBoard;
    }

    // use stack to limit RAM usage... (todo: make configurable?)
    constexpr int32_t USE_STACK_DEPTH = 4;
    constexpr std::size_t STACK_NODE_STACK_CAPACITY = 32; // todo: what should this hard limit be?
    struct stack_node_stack_limit_exceeded : std::logic_error { stack_node_stack_limit_exceeded() : std::logic_error{ "chess::stack_node_stack_limit_exceeded" } {} };

    template <player Player, player Turn, typename Representation>
    double negascout(move_tables<Representation> const& tables, basic_position<Representation>& position, game_tree_node& node, int32_t ply, int32_t depth, double alpha, double beta)
    {
        typedef game_tree_node stack_node_t;
        typedef std::vector<stack_node_t> stack_node_stack_t;
        thread_local stack_node_stack_t stackNodeStack;
        auto const stackUsageDepth = ply - depth;
        bool const useStack = stackUsageDepth >= USE_STACK_DEPTH;
        auto const stackStackIndex = stackUsageDepth - USE_STACK_DEPTH;
        if (useStack && stackNodeStack.size() <= stackStackIndex)
        {
            stackNodeStack.reserve(STACK_NODE_STACK_CAPACITY);
            if (stackStackIndex >= stackNodeStack.capacity())
                throw stack_node_stack_limit_exceeded();
            stackNodeStack.resize(stackStackIndex + 1);
        }
        auto& use = (useStack ? stackNodeStack[stackStackIndex] : node);
        if (use.children == std::nullopt)
        {
            use.children.emplace();
            valid_moves<Turn>(tables, position, use);
        }
        else if (useStack)
            valid_moves<Turn>(tables, position, use);
        auto& validMoves = *use.children;
        if (depth == 0 || validMoves.empty())
        {
            auto result = eval<Representation, Turn>{}(tables, position, static_cast<double>(ply - depth)).eval;
            return result;
        }
        auto a = alpha;
        auto b = beta;
        for (auto& child : validMoves)
        {
            move_piece(position, *child.move);
            double t = -negascout<Player, opponent_v<Turn>>(tables, position, child, ply, depth - 1, -b, -a);
            if (t > a && t < beta && &child != &validMoves[0] && depth > 1)
                a = -negascout<Player, opponent_v<Turn>>(tables, position, child, ply, depth - 1, -beta, -t);
            undo(position);
            a = std::max(a, t);
            if (a >= beta)
                return a;
            b = a + std::numeric_limits<double>::epsilon();
        }
        return a;
    }

    template <typename Representation, player Player>
    ai_thread<Representation, Player>::ai_thread() :
        iMoveTables{ generate_move_tables<representation_type>() },
        iThread{ [&]() { process(); } }
    {
    }

    template <typename Representation, player Player>
    ai_thread<Representation, Player>::~ai_thread()
    {
        {
            std::lock_guard<std::mutex> lk{ iMutex };
            iFinished = true;
        }
        iSignal.notify_one();
        iThread.join();
    }

    template <typename Representation, player Player>
    std::promise<game_tree_node>& ai_thread<Representation, Player>::eval(position_type const& aPosition, game_tree_node&& aNode, int32_t aPly)
    {
        {
            std::lock_guard<std::mutex> lk{ iMutex };
            iQueue.emplace_back(aPosition, std::move(aNode), aPly);
        }
        return iQueue.back().result;
    }

    template <typename Representation, player Player>
    void ai_thread<Representation, Player>::start()
    {
        {
            std::unique_lock<std::mutex> lk{ iMutex };
            if (iQueue.empty())
                return;
        }
        iSignal.notify_one();
    }

    template <typename Representation, player Player>
    void ai_thread<Representation, Player>::process()
    {
        for (;;)
        {
            std::unique_lock<std::mutex> lk{ iMutex };
            iSignal.wait(lk, [&]() { return iFinished || !iQueue.empty(); });
            if (iFinished)
                return;
            for (auto& workItem : iQueue)
            {
                auto& evalPosition = eval_board<Representation>();
                evalPosition = workItem.position;
                auto& node = workItem.node;
                move_piece(evalPosition, *node.move );
                double const signCorrection = (workItem.ply % 2 == 1 ? -1.0 : 1.0);
                node.eval = signCorrection * negascout<Player, opponent_v<Player>>(iMoveTables, evalPosition, node, workItem.ply, workItem.ply, -std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
                workItem.result.set_value(std::move(node));
            }
            iQueue.clear();
        }
    }

    template class ai_thread<mailbox_rep, player::White>;
    template class ai_thread<mailbox_rep, player::Black>;
    template class ai_thread<bitboard_rep, player::White>;
    template class ai_thread<bitboard_rep, player::Black>;
}