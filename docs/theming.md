# Theming in VAXPUI

VAXPUI includes a comprehensive theming system for consistent styling.

## Built-in Themes

### Light Theme (Default)

```c
const VaxpTheme* theme = vaxp_theme_light();
```

### Dark Theme

```c
const VaxpTheme* theme = vaxp_theme_dark();
vaxp_theme_set(theme);
```

## Using the Theme

### Getting Current Theme

```c
const VaxpTheme* theme = vaxp_theme_get_current();

// Access colors
VaxpColor bg = theme->colors.background;
VaxpColor primary = theme->colors.primary;

// Access typography
VaxpTextStyle body = theme->typography.body1;

// Access spacing
VaxpF32 padding = theme->spacing_md;  // 16px
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
VaxpColor primary = { 100, 50, 200, 255 };   // Purple
VaxpColor secondary = { 200, 100, 50, 255 }; // Orange

VaxpTheme my_theme = vaxp_theme_create(primary, secondary, VAXP_FALSE);
vaxp_theme_set(&my_theme);
```

### Full Custom Theme

```c
static VaxpTheme my_theme = {
    .name = "My Theme",
    .is_dark = VAXP_FALSE,
    
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
vaxp_theme_set(&my_theme);
```

## Pre-defined Colors

Material Design colors are available:

```c
VAXP_COLOR_BLUE        // { 33, 150, 243, 255 }
VAXP_COLOR_INDIGO      // { 63, 81, 181, 255 }
VAXP_COLOR_PURPLE      // { 156, 39, 176, 255 }
VAXP_COLOR_PINK        // { 233, 30, 99, 255 }
VAXP_COLOR_RED         // { 244, 67, 54, 255 }
VAXP_COLOR_ORANGE      // { 255, 152, 0, 255 }
VAXP_COLOR_YELLOW      // { 255, 235, 59, 255 }
VAXP_COLOR_GREEN       // { 76, 175, 80, 255 }
VAXP_COLOR_TEAL        // { 0, 150, 136, 255 }
VAXP_COLOR_CYAN        // { 0, 188, 212, 255 }

// Grays
VAXP_COLOR_GRAY_100 through VAXP_COLOR_GRAY_900

// Special
VAXP_COLOR_WHITE
VAXP_COLOR_BLACK
VAXP_COLOR_TRANSPARENT
```

## Dynamic Theme Switching

```c
static VaxpBool is_dark = VAXP_FALSE;

void toggle_theme(void) {
    is_dark = !is_dark;
    vaxp_theme_set(is_dark ? vaxp_theme_dark() : vaxp_theme_light());
    // Trigger UI redraw
}
```

## See Also

- [Widgets](widgets.md) - Using themed widgets
- [API Reference](api-reference.md) - Full theme API
