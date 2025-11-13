# DLC3/K-Line Library Research Findings

## Research Summary

**Date:** 2025-01-27  
**Status:** Research needed - manual search required

---

## What We Know

### From Documentation

1. **OBD9141 Library** - Mentioned in OEM_ECU_DATA_ACCESS.md
   - Arduino library for ISO 9141
   - Status: Existence needs verification
   - ESP32 compatibility: Unknown

2. **TechStream Lite** - Mentioned in multiple docs
   - Open-source Toyota diagnostics
   - Status: Existence needs verification
   - Source code location: Unknown

3. **Your Existing Code** - ecu_obd2_bluetooth.ino
   - Uses ELM327 Bluetooth adapter
   - OBD-II protocol (not K-Line)
   - Works with OBD-II compatible vehicles

---

## What We Need to Find

### Critical Research Items

1. **OBD9141 Library**
   - [ ] Does it exist?
   - [ ] GitHub repository URL
   - [ ] Arduino Library Manager availability
   - [ ] ESP32 compatibility
   - [ ] Last update date
   - [ ] Documentation quality

2. **TechStream Lite**
   - [ ] Source code repository
   - [ ] Programming language
   - [ ] Portability to ESP32
   - [ ] Toyota command structures
   - [ ] Documentation

3. **ESP32 K-Line Projects**
   - [ ] Any existing implementations?
   - [ ] Working examples?
   - [ ] Circuit designs?
   - [ ] Code quality?

4. **Toyota-Specific Projects**
   - [ ] 1KZ-TE projects?
   - [ ] Land Cruiser diagnostic projects?
   - [ ] DLC3 reading projects?

---

## Recommended Search Strategy

### Step 1: GitHub Search

**Search these terms on GitHub:**

1. **"OBD9141"**
   - Look for Arduino libraries
   - Check ESP32 compatibility
   - Review code quality

2. **"TechStream Lite"**
   - Find source code
   - Check language/portability
   - Review Toyota commands

3. **"ESP32 K-Line"**
   - Find existing implementations
   - Check working examples
   - Review circuit designs

4. **"ESP32 ISO 9141"**
   - Alternative search term
   - May find different projects

5. **"Toyota diagnostic"**
   - Broad search
   - May find relevant projects

6. **"1KZ-TE ECU"**
   - Specific to your engine
   - May find exact use case

7. **"DLC3 ESP32"**
   - Specific connector search
   - May find direct implementations

---

### Step 2: Arduino Library Manager

**In Arduino IDE:**
1. Tools → Manage Libraries
2. Search: "OBD9141"
3. Search: "ISO 9141"
4. Search: "K-Line"
5. Check each result:
   - ESP32 compatibility
   - Last update
   - Documentation
   - Examples

---

### Step 3: Forum Searches

**IH8MUD (Land Cruiser Forum):**
- Search: "diagnostic port ESP32"
- Search: "ECU reading Arduino"
- Search: "OBD port reading"
- Search: "TechStream alternative"

**Arduino Forums:**
- Search: "K-Line ESP32"
- Search: "ISO 9141 Arduino"
- Search: "automotive diagnostic ESP32"

**ESP32 Forums:**
- Search: "K-Line"
- Search: "automotive"
- Search: "diagnostic port"

---

### Step 4: PlatformIO Registry

**Search PlatformIO libraries:**
- "OBD9141"
- "ISO 9141"
- "K-Line"
- Check ESP32 compatibility

---

## Expected Findings

### Best Case: OBD9141 Exists

**If OBD9141 library exists and works on ESP32:**
- ✅ Implementation time: 4-6 hours
- ✅ Just install and use
- ✅ Minimal porting needed

**Action:** Install library, test, integrate

---

### Good Case: OBD9141 Needs Porting

**If OBD9141 exists but needs ESP32 porting:**
- ⚠️ Implementation time: 8-12 hours
- ⚠️ Need to modify for ESP32
- ✅ Base library exists

**Action:** Port library, test, integrate

---

### Medium Case: Basic Library + Toyota Commands

**If basic ISO 9141 library exists:**
- ⚠️ Implementation time: 12-18 hours
- ⚠️ Need to add Toyota commands
- ✅ Protocol library exists

**Action:** Use library, add Toyota commands from TechStream Lite

---

### Worst Case: Write from Scratch

**If no libraries exist:**
- ❌ Implementation time: 20-40 hours
- ❌ Start with ISO 9141 spec
- ❌ Reverse-engineer everything

**Action:** Not recommended - use analog intercept instead

---

## Research Checklist

### Immediate Actions

- [ ] Search GitHub for "OBD9141"
- [ ] Search GitHub for "TechStream Lite"
- [ ] Search GitHub for "ESP32 K-Line"
- [ ] Check Arduino Library Manager
- [ ] Search IH8MUD forum
- [ ] Search Arduino forums
- [ ] Document all findings

### Documentation

- [ ] Create list of found libraries
- [ ] Assess ESP32 compatibility
- [ ] Review code quality
- [ ] Check documentation
- [ ] Verify examples exist
- [ ] Update implementation guide

---

## Next Steps After Research

1. **If library found:**
   - Test on ESP32
   - Verify Toyota commands
   - Integrate into project

2. **If partial library found:**
   - Port to ESP32
   - Add Toyota commands
   - Test and integrate

3. **If no library found:**
   - Reassess approach
   - Consider analog intercept
   - Or write minimal K-Line code

---

## Research Resources

### GitHub Search URLs

```
https://github.com/search?q=OBD9141
https://github.com/search?q=TechStream+Lite
https://github.com/search?q=ESP32+K-Line
https://github.com/search?q=ESP32+ISO+9141
https://github.com/search?q=Toyota+diagnostic
https://github.com/search?q=1KZ-TE+ECU
https://github.com/search?q=DLC3+ESP32
```

### Forums

- **IH8MUD:** https://forum.ih8mud.com
- **Arduino Forums:** https://forum.arduino.cc
- **ESP32 Forums:** https://www.esp32.com

### Documentation

- **ISO 9141-2 Standard:** Search for PDF
- **MC33290 Datasheet:** Available from manufacturers
- **Toyota Service Manuals:** For command reference

---

**Document Status:** Research template - needs manual search  
**Last Updated:** 2025-01-27
