from enum import Enum, auto

class UIPageType(Enum):
    MAIN = auto()

class UIPage:
    def __init__(self, page_type: UIPageType):
        self.page_type = page_type

    def render(self, screen):
        pass

class MainCalculatorPage(UIPage):
    def __init__(self):
        super().__init__(UIPageType.MAIN)

    def render(self, screen):
        pass