# Genius Square Difficulty Classifier

A suite of C++ tools to count puzzle solutions and classify them by difficulty.

## Overview

This toolset analyzes all 100 possible Genius Square puzzle arrangements (seeds 0-99) and determines their difficulty based on solution count. The results are packed into a compact 25-byte lookup table for the embedded firmware.

## Tools

### `genius_solver`
Counts the total number of solutions for each puzzle seed.

**Usage:**
```bash
./genius_solver
```

**Output:**
- Writes raw solution counts to `results.txt` (one per line)
- Example: seed 0 has 88 solutions, seed 1 has 863 solutions
- Takes ~5 minutes total (most seeds complete in <10s)

### `classifier`
Categorizes solution counts into three difficulty bands.

**Usage:**
```bash
./classifier
```

**Input:** `results.txt` (from genius_solver)

**Output:** `classifications.txt` (one digit per line: 1=HARD, 2=MEDIUM, 3=EASY)

**Thresholds (customizable in source):**
- Band 1 (HARD): ≤120 solutions
- Band 2 (MEDIUM): 121-1000 solutions
- Band 3 (EASY): >1000 solutions

### `generate_difficulty_data`
Generates packed binary data for embedding in firmware.

**Usage:**
```bash
./generate_difficulty_data
```

**Input:** `classifications.txt` (from classifier)

**Output:** C hex array (100 classifications packed into 25 bytes)

**Format:**
- 2 bits per seed (4 classifications per byte)
- Direct bit values: 1=HARD, 2=MEDIUM, 3=EASY, 0=INVALID

## Workflow

1. **Count solutions:**
   ```bash
   ./genius_solver      # → results.txt
   ```

2. **Classify by difficulty:**
   ```bash
   ./classifier         # → classifications.txt
   ```

3. **Generate firmware data:**
   ```bash
   ./generate_difficulty_data  # → hex array for C header
   ```

4. **Integrate into firmware:**
   - Copy the hex array output
   - Paste into `DifficultyLookup.cpp` DIFFICULTY_DATA array
   - Rebuild firmware with `pio run`

## Data Structure

The packed data uses 2 bits per seed for compact storage:

```
DIFFICULTY_DATA[25] = {
    0xbd,  // seeds 0-3:  bits [01, 11, 11, 10] = [1, 3, 3, 2]
    0xef,  // seeds 4-7:  bits [01, 01, 11, 11] = [1, 1, 3, 3]
    ...
}
```

Each byte stores 4 seeds (bits 0-1, 2-3, 4-5, 6-7).

## Customization

Edit `classifier.cpp` to adjust difficulty thresholds:

```cpp
static const int THRESHOLD_1 = 120;  // Hard/Medium boundary
static const int THRESHOLD_2 = 1000; // Medium/Easy boundary
```

Then rebuild and re-run the classifier.

## Rebuild

```bash
make clean && make
```

Compiles all three tools with `-O2` optimization.
