# Theming in VENOMUI

VENOMUI includes a comprehensive theming system for consistent styling.

## Built-in Themes

### Light Theme (Default)

```c
const VenomTheme* theme = venom_theme_light();
```

### Dark Theme

```c
const VenomTheme* theme = venom_theme_dark();
venom_theme_set(theme);
```

## Using the Theme

### Getting Current Theme

```c
const VenomTheme* theme = venom_theme_get_current();

// Access colors
VenomColor bg = theme->colors.background;
VenomColor primary = theme->colors.primary;

// Access typography
VenomTextStyle body = theme->typography.body1;

// Access spacing
VenomF32 padding = theme->spacing_md;  // 16px
```

### Color Palette

| Color | Description |
|-------|-------------|
| `primary` | Main brand color |
| `primary_variant` | Darker primary |
| `on_primary` | Text on primary |
| `secondary` | Accent color |
| `background` | App background |
| `surface` | Card/dialog background |
| `on_surface` | Text on surface |
| `error` | Error state |
| `success` | Success state |
| `warning` | Warning state |

### Typography Scale

| Style | Size | Usage |
|-------|------|-------|
| `h1` | 96px | Large display |
| `h2` | 60px | Display |
| `h3` | 48px | Heading |
| `h4` | 34px | Heading |
| `h5` | 24px | Heading |
| `h6` | 20px | Section |
| `body1` | 16px | Main text |
| `body2` | 14px | Secondary text |
| `caption` | 12px | Labels |
| `button` | 14px | Buttons |

### Spacing

```c
theme->spacing_xs  // 4px
theme->spacing_sm  // 8px
theme->spacing_md  // 16px
theme->spacing_lg  // 24px
theme->spacing_xl  // 32px
```

### Border Radii

```c
theme->radius_sm  // 4px
theme->radius_md  // 8px
theme->radius_lg  // 16px
```

## Creating Custom Themes

### Quick Theme from Colors

```c
VenomColor primary = { 100, 50, 200, 255 };   // Purple
VenomColor secondary = { 200, 100, 50, 255 }; // Orange

VenomTheme my_theme = venom_theme_create(primary, secondary, VENOM_FALSE);
venom_theme_set(&my_theme);
```

### Full Custom Theme

```c
static VenomTheme my_theme = {
    .name = "My Theme",
    .is_dark = VENOM_FALSE,
    
    .colors = {
        .primary = { 100, 150, 200, 255 },
        .primary_variant = { 80, 120, 180, 255 },
        .on_primary = { 255, 255, 255, 255 },
        
        .secondary = { 200, 100, 150, 255 },
        .secondary_variant = { 180, 80, 130, 255 },
        .on_secondary = { 255, 255, 255, 255 },
        
        .background = { 250, 250, 250, 255 },
        .surface = { 255, 255, 255, 255 },
        .on_background = { 33, 33, 33, 255 },
        .on_surface = { 33, 33, 33, 255 },
        
        .error = { 200, 50, 50, 255 },
        .success = { 50, 200, 50, 255 },
        .warning = { 255, 180, 0, 255 },
    },
    
    .button = {
        .corner_radius = 8.0f,
        .padding_horizontal = 16.0f,
        .padding_vertical = 8.0f,
    },
    
    .input = {
        .corner_radius = 4.0f,
        .border_width = 1.0f,
    },
    
    .spacing_sm = 8.0f,
    .spacing_md = 16.0f,
    .spacing_lg = 24.0f,
    
    .radius_sm = 4.0f,
    .radius_md = 8.0f,
    .radius_lg = 16.0f,
};

// Apply theme
venom_theme_set(&my_theme);
```

## Pre-defined Colors

Material Design colors are available:

```c
VENOM_COLOR_BLUE        // { 33, 150, 243, 255 }
VENOM_COLOR_INDIGO      // { 63, 81, 181, 255 }
VENOM_COLOR_PURPLE      // { 156, 39, 176, 255 }
VENOM_COLOR_PINK        // { 233, 30, 99, 255 }
VENOM_COLOR_RED         // { 244, 67, 54, 255 }
VENOM_COLOR_ORANGE      // { 255, 152, 0, 255 }
VENOM_COLOR_YELLOW      // { 255, 235, 59, 255 }
VENOM_COLOR_GREEN       // { 76, 175, 80, 255 }
VENOM_COLOR_TEAL        // { 0, 150, 136, 255 }
VENOM_COLOR_CYAN        // { 0, 188, 212, 255 }

// Grays
VENOM_COLOR_GRAY_100 through VENOM_COLOR_GRAY_900

// Special
VENOM_COLOR_WHITE
VENOM_COLOR_BLACK
VENOM_COLOR_TRANSPARENT
```

## Dynamic Theme Switching

```c
static VenomBool is_dark = VENOM_FALSE;

void toggle_theme(void) {
    is_dark = !is_dark;
    venom_theme_set(is_dark ? venom_theme_dark() : venom_theme_light());
    // Trigger UI redraw
}
```

## See Also

- [Widgets](widgets.md) - Using themed widgets
- [API Reference](api-reference.md) - Full theme API
