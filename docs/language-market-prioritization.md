# Language Market Prioritization

## Executive summary

This document prioritizes language packs for GTAVAddonLoader by combining three different signals:

1. active video game players in major national markets;
2. reported preference for single-player games; and
3. the language distribution of the PC-focused Steam audience.

The country-level single-player figures below are estimates, not observed counts of Grand Theft Auto V players or mod users. The selected release adds Russian, Spanish for Spain and Latin America, Brazilian Portuguese, German, Japanese, and French. Together with the existing English and Simplified Chinese packs, these languages represent approximately **88.99%** of the June 2026 Steam language survey.

The translations are complete and structurally validated, but they have not been reviewed by native-speaking localization professionals.

## Market definition and method

Newzoo publishes active-player estimates for the ten highest-revenue game markets in its 2025 ranking. These totals include mobile, PC, and console players and therefore provide a broad market-size signal rather than a direct estimate of this Windows PC mod's audience.

Ampere Analysis reported that 56% of surveyed players across 22 markets preferred single-player games. The same study reported country-specific results of 65% for the United States and 47% for China. The estimated single-player audience is calculated as:

```text
Estimated single-player audience = active game players × single-player preference
```

The United States and China use their reported country-specific preference rates. All other countries use the 56% multi-market benchmark because comparable country-specific rates were not publicly available. For those proxy-based estimates, the sensitivity range applies the observed China-to-United-States span of 47% to 65%.

## Estimated single-player audience in major markets

| Country | Active game players, 2025 | Preference used | Estimated single-player audience | Sensitivity range | Evidence type |
|---|---:|---:|---:|---:|---|
| China | 723.0M | 47% | 339.8M | Not modeled | Country-specific survey rate |
| United States | 224.8M | 65% | 146.1M | Not modeled | Country-specific survey rate |
| Brazil | 123.3M | 56% | 69.0M | 58.0M–80.1M | Multi-market proxy |
| Japan | 74.1M | 56% | 41.5M | 34.8M–48.2M | Multi-market proxy |
| Germany | 53.2M | 56% | 29.8M | 25.0M–34.6M | Multi-market proxy |
| United Kingdom | 43.4M | 56% | 24.3M | 20.4M–28.2M | Multi-market proxy |
| France | 40.2M | 56% | 22.5M | 18.9M–26.1M | Multi-market proxy |
| Italy | 37.1M | 56% | 20.8M | 17.4M–24.1M | Multi-market proxy |
| South Korea | 34.0M | 56% | 19.0M | 16.0M–22.1M | Multi-market proxy |
| Canada | 24.4M | 56% | 13.7M | 11.5M–15.9M | Multi-market proxy |

The ordering above is based on the resulting audience estimate, not Newzoo's revenue rank. These ten countries are the markets exposed by Newzoo's public top-revenue ranking; the table is not a complete global country ranking.

## Steam language evidence

Steam is a closer proxy for this project than total gamer population because GTAVAddonLoader is a Windows PC mod. The Steam Hardware & Software Survey is still an optional survey and measures client language rather than nationality, ownership of GTA V, interest in single-player games, or mod installation behavior.

| Steam client language | June 2026 share | Project status | Decision |
|---|---:|---|---|
| English | 38.18% | Existing `en-US` | Retain |
| Simplified Chinese | 24.19% | Existing `zh-CN` | Retain |
| Russian | 9.61% | Added `ru-RU` | Include in this batch |
| Spanish — Spain | 4.59% | Added `es-ES` | Include in this batch |
| Portuguese — Brazil | 4.00% | Added `pt-BR` | Include in this batch |
| German | 2.71% | Added `de-DE` | Include in this batch |
| Japanese | 2.64% | Added `ja-JP` | Include in this batch |
| French | 2.26% | Added `fr-FR` | Include in this batch |
| Polish | 1.67% | Not included | Candidate for the next batch |
| Traditional Chinese | 1.50% | Not included | Candidate for the next batch |
| Korean | 1.48% | Not included | Candidate for the next batch |
| Turkish | 1.32% | Not included | Candidate for the next batch |
| Spanish — Latin America | 0.81% | Added `es-419` | Include alongside Spanish coverage |
| Italian | 0.61% | Not included | Candidate for the next batch |

The selected coverage is calculated directly from the Steam shares:

```text
38.18 + 24.19 + 9.61 + 4.59 + 0.81 + 4.00 + 2.71 + 2.64 + 2.26 = 88.99%
```

This percentage describes represented Steam client languages only. It is not a projected adoption rate, revenue share, or count of reachable users.

## Prioritization decision

| Language or market | National-market signal | Steam signal | Implementation decision |
|---|---|---:|---|
| English | United States, United Kingdom, and Canada together represent a large estimated single-player audience | 38.18% | Already supported |
| Simplified Chinese | China is the largest listed player market even with the lowest reported single-player preference | 24.19% | Already supported |
| Brazilian Portuguese | Brazil has an estimated 69.0M single-player players under the proxy model | 4.00% | Add `pt-BR` |
| Japanese | Japan has an estimated 41.5M single-player players | 2.64% | Add `ja-JP` |
| German | Germany has an estimated 29.8M single-player players | 2.71% | Add `de-DE` |
| French | France has an estimated 22.5M single-player players; French also serves additional markets not quantified here | 2.26% | Add `fr-FR` |
| Russian | Russia is not exposed in Newzoo's public top-ten revenue table, but Russian is the largest unsupported Steam language | 9.61% | Add `ru-RU` |
| Spanish | The public country table does not provide a complete Spanish-speaking market total, but Spanish is the second-largest unsupported language group | 5.40% combined | Add `es-ES` and `es-419` |

Country scale supports Brazilian Portuguese, Japanese, German, and French. Steam usage adds strong evidence for Russian and Spanish, which would be missed by a strict reading of the public top-ten revenue-market table.

## Deferred languages

The next localization batch should consider:

- **Polish (`pl-PL`)** — 1.67% Steam share.
- **Traditional Chinese (`zh-TW`)** — 1.50% Steam share.
- **Korean (`ko-KR`)** — 1.48% Steam share and a 34.0M-player national market in the Newzoo table.
- **Turkish (`tr-TR`)** — 1.32% Steam share.
- **Italian (`it-IT`)** — 0.61% Steam share but an estimated 20.8M single-player players under the proxy model.

Actual project download telemetry by locale would be the most valuable input for reordering this list. Without that telemetry, Steam language share remains the strongest available PC-specific proxy.

## Limitations and review requirements

- Newzoo player totals include mobile, PC, and console gaming; they are not PC-only totals.
- The Newzoo public table covers the ten highest-revenue markets, not every country ranked by player count.
- The 56% rate is a multi-market preference result, not an observed rate for every country to which it is applied.
- A preference for single-player games does not prove ownership of GTA V or use of vehicle mods.
- Steam survey participation is optional and anonymous, and results can move materially from month to month.
- Steam client language is not equivalent to country of residence.
- The 88.99% figure is a sum of language shares, not a forecast of translation usage.
- The new translations require native-speaker review for tone, terminology, menu width, and in-game rendering before they should be described as professionally localized.

## Sources

Sources were retrieved on July 13, 2026.

- Newzoo, [Top countries and markets by video game revenues](https://newzoo.com/resources/rankings/top-10-countries-by-game-revenues), 2025 estimates. Newzoo states that its estimates combine primary consumer research, transactional data, company reports, and census data.
- Valve, [Steam Hardware & Software Survey](https://store.steampowered.com/hwsurvey/us/?l=english), June 2026. Valve states that participation is optional and anonymous.
- GameSpot summary of Ampere Analysis, [Analysts Say More Than Half Of Gamers Prefer Single-Player Games](https://www.gamespot.com/articles/analysts-more-than-half-of-gamers-prefer-single-player-games/1100-6536314/), November 18, 2025. The reported survey covered 34,000 players across 22 markets.
