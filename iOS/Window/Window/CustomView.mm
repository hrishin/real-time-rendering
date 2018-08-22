//
//  CustomView.m
//  Window
//
//  Created by Hrishikesh Shinde on 22/08/18.
//

#import "CustomView.h"

@implementation CustomView
{
    NSString *centerText;
}

- (id) initWithFrame: (CGRect)frameRect
{
    self = [super initWithFrame: frameRect];
    if(self)
    {
        /// background color
        [self setBackgroundColor:[UIColor whiteColor]];
        
        centerText =  @"Hello World!!";
        
        // gesture handling
        UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onSingleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [singleTap setDelegate: self];
        [self addGestureRecognizer: singleTap];
        
        UITapGestureRecognizer *doubleTap = [[UITapGestureRecognizer alloc] initWithTarget: self action: @selector(onDoubleTap:)];
        [singleTap setNumberOfTapsRequired: 1];
        [singleTap setNumberOfTouchesRequired: 1]; // 1 finger
        [singleTap setDelegate: self];
        [self addGestureRecognizer: doubleTap];
        
        //  let single ignore it if its double tapped
        [singleTap requireGestureRecognizerToFail:doubleTap];
        
        // swipe gesture
        UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget: self action: @selector(onSwipe:)];
        [self addGestureRecognizer: swipe];
        
        // long press
        UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc]initWithTarget: self action: @selector(onLongPress:)];
        [self addGestureRecognizer: longPressGestureRecognizer];
    }
    
    return self;
}

// override if want to do custom drawing
// empty implemntation may impact rendering performance
- (void) drawRect: (CGRect)rect
{
    UIColor *fillColor =[UIColor blackColor];
    [fillColor set];
    UIRectFill(rect);
    
    NSDictionary *textAttributes = [NSDictionary dictionaryWithObjectsAndKeys: [UIFont fontWithName: @"Helvetica" size: 24], NSFontAttributeName, [UIColor greenColor], NSForegroundColorAttributeName, nil];
    
    CGSize textSize = [centerText sizeWithAttributes: textAttributes];
    
    CGPoint point;
    point.x = (rect.size.width/2) - (textSize.width/2);
    point.y = (rect.size.height/2) - (textSize.height/2) + 12;
    
    [centerText drawAtPoint: point withAttributes: textAttributes];
}

-(BOOL) acceptsFirstResponder
{
    return YES;
}

- (void) touchesBegan: (NSSet *)touches withEvent: (UIEvent *)event
{
     centerText = @"'touchesBegan' Event Occured";
     [self setNeedsDisplay];
}

- (void) onSingleTap: (UITapGestureRecognizer *)gr
{
    NSLog(@"single tapped");
    centerText = @"'onSingleTap' Event Occured";
    [self setNeedsDisplay];
}

- (void) onDoubleTap: (UITapGestureRecognizer*)gr
{
    centerText = @"'onDoubleTap' Event Occured";
    [self setNeedsDisplay];
}

- (void) onSwipe: (UITapGestureRecognizer*)gr
{
    [self release];
    exit(0);
}

- (void) onLongPress: (UILongPressGestureRecognizer*)gr
{
    centerText = @"'onLongPress' Event Occured";
    [self setNeedsDisplay];
}

- (void) dealloc
{
    [super dealloc];
}
@end
