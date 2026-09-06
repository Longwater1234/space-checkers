//
// BotPersona.hpp — the voice of "Ibox Offline".
//
// The commentary is not random flavour text: which line comes out is decided by
// what the search actually found (score swing, whether the human played the move
// on the principal variation, chain length, material, phase). That is what makes
// the sarcasm land — the bot only calls something a blunder when it *is* one.
//
// Style note: Ibox is an homage to the grinding, never-offer-a-draw endgame
// world-champion archetype. It is its own character with its own lines, not an
// impersonation of any real player.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace chk::bot
{

inline constexpr auto BOT_NAME = "IBOX OFFLINE";
inline constexpr auto BOT_TAGLINE = "endgame grinder";

/// What Ibox is reacting to. Higher entries win when several apply at once.
enum class Mood : uint8_t
{
    NONE = 0,
    GREETING,
    QUIET,         // played a quiet move
    CAPTURE,       // took one piece
    MULTI_CAPTURE, // took a chain
    CROWNED,       // promoted
    PREDICTED,     // human played the exact move on our PV
    BLUNDER,       // human's move cost them real material
    GOOD_MOVE,     // human found something better than we expected
    DOMINATING,    // far ahead
    STRUGGLING,    // behind
    HUMAN_SLOW,    // human took ages
    HUMAN_CROWNED, // human promoted
    HUMAN_FORCED,  // human is now obliged to capture
    ENDGAME_GRIND, // level and nearly empty, but playing on regardless
    WIN,
    LOSS,
    DRAW,
    COUNT
};

/**
 * Everything the manager knows about the turn that just finished. Fed straight
 * into decide() so mood selection stays in one place.
 */
struct TurnReport
{
    bool gameJustStarted{false};
    bool gameOver{false};
    bool botWon{false};
    bool drawn{false};

    int botCaptures{0}; // pieces taken by this bot move
    bool botPromoted{false};
    bool humanPromoted{false};
    bool humanMustCapture{false};
    bool humanPlayedPredicted{false};

    /// Change in evaluation caused by the human's last move, from the bot's
    /// point of view, in centi-men. Positive == the human made things worse.
    int humanScoreSwing{0};
    /// Current evaluation from the bot's point of view, in centi-men.
    int scoreCp{0};
    int piecesLeft{24};
    int humanThinkMs{0};
};

/**
 * Picks lines without repeating: each bank is a shuffle bag that reshuffles
 * only when exhausted, so you never hear the same quip twice in a row.
 */
class BotPersona
{
  public:
    explicit BotPersona(uint32_t seed = 0)
    {
        if (seed == 0)
        {
            std::random_device rd;
            seed = rd();
        }
        m_rng.seed(seed);
        buildBanks();
        for (auto &bag : m_bags)
        {
            reshuffle(bag);
        }
    }

    /**
     * Choose what Ibox should react to. Ordering below is the priority: game
     * result first, then anything dramatic, then general position assessment.
     */
    [[nodiscard]] static Mood decide(const TurnReport &r)
    {
        if (r.gameOver)
        {
            if (r.drawn)
            {
                return Mood::DRAW;
            }
            return r.botWon ? Mood::WIN : Mood::LOSS;
        }
        if (r.gameJustStarted)
        {
            return Mood::GREETING;
        }
        if (r.botCaptures >= 2)
        {
            return Mood::MULTI_CAPTURE;
        }
        if (r.botPromoted)
        {
            return Mood::CROWNED;
        }
        // A real blunder outranks a single capture — the capture is the symptom.
        if (r.humanScoreSwing >= BLUNDER_CP)
        {
            return Mood::BLUNDER;
        }
        if (r.botCaptures == 1)
        {
            return Mood::CAPTURE;
        }
        if (r.humanScoreSwing <= -GOOD_MOVE_CP)
        {
            return Mood::GOOD_MOVE;
        }
        if (r.humanPromoted)
        {
            return Mood::HUMAN_CROWNED;
        }
        if (r.humanThinkMs >= SLOW_MS)
        {
            return Mood::HUMAN_SLOW;
        }
        if (r.humanMustCapture)
        {
            return Mood::HUMAN_FORCED;
        }
        // The signature situation: dead level, almost no pieces, still playing on.
        if (r.piecesLeft <= 8 && std::abs(r.scoreCp) < 60)
        {
            return Mood::ENDGAME_GRIND;
        }
        if (r.scoreCp >= DOMINATING_CP)
        {
            return Mood::DOMINATING;
        }
        if (r.scoreCp <= -DOMINATING_CP)
        {
            return Mood::STRUGGLING;
        }
        if (r.humanPlayedPredicted)
        {
            return Mood::PREDICTED;
        }
        return Mood::QUIET;
    }

    /// Should Ibox say anything at all this turn? Quiet moves are only sometimes
    /// worth a remark; anything notable always is.
    [[nodiscard]] bool shouldSpeak(Mood m)
    {
        if (m == Mood::NONE)
        {
            return false;
        }
        if (m == Mood::QUIET)
        {
            return (m_rng() % 100) < 55;
        }
        if (m == Mood::ENDGAME_GRIND || m == Mood::DOMINATING || m == Mood::STRUGGLING)
        {
            return (m_rng() % 100) < 70;
        }
        return true;
    }

    /// Draw the next unused line from this mood's bag.
    const std::string &speak(Mood m)
    {
        auto &bag = m_bags[static_cast<size_t>(m)];
        if (bag.lines.empty())
        {
            return m_fallback;
        }
        if (bag.next >= bag.order.size())
        {
            reshuffle(bag);
        }
        return bag.lines[bag.order[bag.next++]];
    }

  private:
    static constexpr int BLUNDER_CP = 90;     // a swing worth mocking
    static constexpr int GOOD_MOVE_CP = 70;   // a swing worth respecting
    static constexpr int DOMINATING_CP = 220; // roughly two men up
    static constexpr int SLOW_MS = 20000;

    struct Bag
    {
        std::vector<std::string> lines;
        std::vector<size_t> order;
        size_t next{0};
    };

    std::array<Bag, static_cast<size_t>(Mood::COUNT)> m_bags;
    std::mt19937 m_rng;
    std::string m_fallback{"..."};

    void reshuffle(Bag &bag)
    {
        const size_t previous = (bag.order.empty() || bag.next == 0) ? SIZE_MAX : bag.order[bag.next - 1];
        bag.order.resize(bag.lines.size());
        for (size_t i = 0; i < bag.order.size(); ++i)
        {
            bag.order[i] = i;
        }
        std::shuffle(bag.order.begin(), bag.order.end(), m_rng);
        // Avoid repeating the last line across a reshuffle boundary.
        if (bag.order.size() > 1 && bag.order[0] == previous)
        {
            std::swap(bag.order[0], bag.order[bag.order.size() - 1]);
        }
        bag.next = 0;
    }

    void set(Mood m, std::vector<std::string> lines)
    {
        m_bags[static_cast<size_t>(m)].lines = std::move(lines);
    }

    void buildBanks()
    {
        set(Mood::GREETING, {
                                "Ibox Offline. Board's set. Sympathies in advance.",
                                "I don't need to beat you quickly. I just need to not lose for forty moves.",
                                "Let's get this over with. Slowly.",
                                "I've already looked at the next fourteen moves. You'll enjoy about two.",
                                "No pressure. It's only a game. For you.",
                                "Sit comfortably. This tends to take a while.",
                            });

        set(Mood::QUIET, {
                             "There. Nothing dramatic. Dramatic comes later.",
                             "A quiet move. The good ones usually are.",
                             "I'm barely doing anything. That's rather the point.",
                             "Improving the position. You wouldn't notice.",
                             "Small move, long consequences.",
                             "Patience. I have an enormous amount of it.",
                             "Squeezing. Gently.",
                             "I could have taken something. I'd rather you worried about it.",
                             "Nothing to see here. Keep telling yourself that.",
                         });

        set(Mood::CAPTURE, {
                               "Yes, that was hanging. For three moves.",
                               "Thank you. I'll take that.",
                               "You left it there. I assumed it was a gift.",
                               "One down. I'm counting, in case you aren't.",
                               "I did warn you. Not out loud, but still.",
                               "Removed. It wasn't doing much anyway.",
                           });

        set(Mood::MULTI_CAPTURE, {
                                     "Two for the price of one. The price was your attention.",
                                     "I'd apologise, but you built that yourself.",
                                     "That was a chain. You may want to sit down.",
                                     "Three moves ago this was already over. Nobody told you.",
                                     "Lovely. Absolutely lovely. For me.",
                                     "I hope you weren't attached to those.",
                                 });

        set(Mood::CROWNED, {
                               "Crowned. The endgame is my part of the board.",
                               "King me. Or rather — already done.",
                               "Now I can go backwards. That should worry you.",
                               "This is the phase where I stop being polite.",
                               "A king. Everything gets slower and worse for you now.",
                           });

        set(Mood::PREDICTED, {
                                 "Yes. That's what I had written down.",
                                 "Expected. Moving on.",
                                 "You played the move I planned around. Convenient.",
                                 "Predictable isn't an insult. It's just accurate.",
                                 "I'd already replied to that in my head.",
                             });

        set(Mood::BLUNDER, {
                               "Oh. Oh dear.",
                               "You'll want that one back. You can't have it.",
                               "That's the move I've been waiting for since move six.",
                               "Bold. Wrong, but bold.",
                               "I didn't even calculate that one. Because it loses.",
                               "Interesting. Not good. But interesting.",
                               "That piece had a family.",
                               "Take a moment. I'll still be winning when you're done.",
                           });

        set(Mood::GOOD_MOVE, {
                                 "Hm. Fine. That's actually fine.",
                                 "Better than I expected. Don't get comfortable.",
                                 "Alright. You've been practising.",
                                 "Noted. I'll take you slightly more seriously. For one move.",
                                 "That's the first thing you've done that I had to check twice.",
                             });

        set(Mood::DOMINATING, {
                                  "I'm up material and I'm up time. Pick a lane to lose in.",
                                  "At this point I'm just being thorough.",
                                  "You can resign. I'd respect it. Slightly.",
                                  "This stopped being a game a while ago. It's a demonstration.",
                                  "I'll finish it properly. It's rude not to.",
                              });

        set(Mood::STRUGGLING, {
                                  "Fine. You've got something. I've been in worse.",
                                  "Don't celebrate. I'm very hard to finish off.",
                                  "I'm worse. I'm also not going anywhere.",
                                  "This is where most people relax. Please do.",
                                  "Down material, up stubbornness.",
                              });

        set(Mood::HUMAN_SLOW, {
                                  "Take your time. I'm not aging.",
                                  "Still there? The move doesn't improve with age.",
                                  "I've looked at four million positions since you touched that piece.",
                                  "There's no move that saves this. But do keep looking.",
                              });

        set(Mood::HUMAN_CROWNED, {
                                     "Congratulations. It's a king. It changes very little.",
                                     "You got one crowned. Consider it a loan.",
                                     "Nice. Now try using it.",
                                 });

        set(Mood::HUMAN_FORCED, {
                                    "You have to take it. That was rather the idea.",
                                    "Capture's compulsory. I chose which one.",
                                    "Go on. It isn't optional.",
                                });

        set(Mood::ENDGAME_GRIND, {
                                     "Most people offer a draw here. I'm not most people.",
                                     "Two pieces and a plan. That's all I've ever needed.",
                                     "This is dead level. I'll win it anyway.",
                                     "The position is drawn. Positions don't play themselves.",
                                     "We can be here a while. I've cleared my schedule.",
                                 });

        set(Mood::WIN, {
                           "Game. Thank you for the exercise.",
                           "That's that. Rematch? For your sake, I mean.",
                           "Well played. I'm lying, but well played.",
                           "I'll be here. Offline, as advertised.",
                       });

        set(Mood::LOSS, {
                            "...Right. Well. That happened.",
                            "You won. I'd like it noted that I made you work.",
                            "Congratulations. I mean that. Once.",
                            "Enjoy it. I don't forget positions.",
                        });

        set(Mood::DRAW, {
                            "A draw. I feel nothing, but I feel it strongly.",
                            "Split point. I'll take it. Reluctantly.",
                            "Nobody wins. I dislike that phrase.",
                        });
    }
};

} // namespace chk::bot
