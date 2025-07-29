#!/usr/bin/env python3
import csv
import argparse
from collections import Counter, defaultdict

def parse_args():
    p = argparse.ArgumentParser(
        description="Aggregate per‑tile channel‑group frequencies into a global summary."
    )
    p.add_argument(
        "input_csv",
        help="Path to the CSV with per‑tile frequencies (needs 'channels' and 'count' columns)."
    )
    p.add_argument(
        "output_csv",
        help="Path where the aggregated global CSV will be written."
    )
    p.add_argument(
        "--with-top-tiles",
        action="store_true",
        help="If set, also include a column with the top 10 tiles per channel group."
    )
    return p.parse_args()

def decode(code, ch_list):
    return "|".join(ch_list[i] for i in range(len(ch_list)) if (code >> i) & 1)

def main():
    args = parse_args()

    
    total_counts = Counter()
    if args.with_top_tiles:
        tiles_by_group = defaultdict(list)

    with open(args.input_csv, newline="") as f:
        reader = csv.DictReader(f)
        
        for row in reader:
            group = row["channels"]
            cnt   = int(float(row["count"]))
            total_counts[group] += cnt

            if args.with_top_tiles:
                x0 = row["x0"]; x1 = row["x1"]
                y0 = row["y0"]; y1 = row["y1"]
                tiles_by_group[group].append((cnt, x0, x1, y0, y1))

    # write output
    with open(args.output_csv, "w", newline="") as f:
        writer = csv.writer(f)

        if args.with_top_tiles:
            writer.writerow(["channels", "count", "top_tiles"])
        else:
            writer.writerow(["channels", "count"])

        for group, total in total_counts.most_common():
            if args.with_top_tiles:
                # order tiles by frequencies
                top = sorted(tiles_by_group[group], key=lambda x: x[0], reverse=True)
                # format as x0-x1:y0-y1(count)
                formatted = [f"{x0}-{x1}:{y0}-{y1}({cnt})" for cnt, x0, x1, y0, y1 in top]
                top_str = ";".join(formatted)
                writer.writerow([group, total, top_str])
            else:
                writer.writerow([group, total])

if __name__ == "__main__":
    main()
