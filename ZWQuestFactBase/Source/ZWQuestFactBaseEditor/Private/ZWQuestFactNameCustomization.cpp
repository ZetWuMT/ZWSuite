// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWQuestFactNameCustomization.h"
#include "DetailWidgetRow.h"
#include "ZWQuestFact.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"

TSharedRef<IPropertyTypeCustomization> FZWQuestFactNameCustomization::MakeInstance()
{
    return MakeShareable(new FZWQuestFactNameCustomization());
}

void FZWQuestFactNameCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    this->PropertyHandlePtr = InPropertyHandle;

    if (PropertyHandlePtr->HasMetaData(TEXT("GetOptions")))
    {
        const FString FunctionName = PropertyHandlePtr->GetMetaData(TEXT("GetOptions"));
        
        TArray<UObject*> OuterObjects;
        PropertyHandlePtr->GetOuterObjects(OuterObjects);
        
        UObject* OwnerObject = (OuterObjects.Num() > 0) ? OuterObjects[0] : nullptr;

        if (OwnerObject)
        {
            // Find the function by name in the owner object
            UFunction* Func = OwnerObject->FindFunction(FName(*FunctionName));
            if (Func && Func->NumParms == 1 && Func->GetReturnProperty())
            {
                // 1. Change the variable type to TArray<FName> to match the function
                TArray<FName> ResultArray;
    
                // 2. Call the function, passing the address of our variable
                OwnerObject->ProcessEvent(Func, &ResultArray);

                // 3. Fill the options list, converting FName to FString for the widget
                for (const FName& Option : ResultArray)
                {
                    OptionsSource.Add(MakeShareable(new FString(Option.ToString())));
                }
            }
        }
    }
    FilteredOptions = OptionsSource;

    HeaderRow
    .NameContent()
    [
        PropertyHandlePtr->CreatePropertyNameWidget()
    ]
    .ValueContent()
    [
        SAssignNew(ComboButton, SComboButton)
        .OnGetMenuContent(FOnGetContent::CreateLambda([this]() {
            return SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SAssignNew(SearchBar, SEditableTextBox)
                    .OnTextChanged(this, &FZWQuestFactNameCustomization::OnSearchTextChanged)
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SAssignNew(ListView, SListView<TSharedPtr<FString>>)
                    .ListItemsSource(&FilteredOptions)
                    .OnGenerateRow(this, &FZWQuestFactNameCustomization::OnGenerateRow)
                    .OnSelectionChanged(this, &FZWQuestFactNameCustomization::OnSelectionChanged)
                    
                ];
        }))
        .ContentPadding(FMargin(4.0, 2.0))
        .ButtonContent()
        [
            SNew(STextBlock)
            .Text(this, &FZWQuestFactNameCustomization::GetCurrentSelectionText)
        ]
        .OnMenuOpenChanged(this, &FZWQuestFactNameCustomization::OnMenuOpenChanged)
    ];
}

void FZWQuestFactNameCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
    IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FZWQuestFactNameCustomization::OnSearchTextChanged(const FText& InText)
{
    FString SearchString = InText.ToString();
    FilteredOptions.Empty();

    if (SearchString.IsEmpty())
    {
        FilteredOptions = OptionsSource;
    }
    else
    {
        for (const TSharedPtr<FString>& Option : OptionsSource)
        {
            if (Option->Contains(SearchString))
            {
                FilteredOptions.Add(Option);
            }
        }
    }
    ListView->RequestListRefresh();
}

void FZWQuestFactNameCustomization::OnSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (NewSelection.IsValid())
    {
        TSharedPtr<IPropertyHandle> QuestFactNameHandle = PropertyHandlePtr->GetChildHandle(GET_MEMBER_NAME_CHECKED(FZWQuestFactSearchableName, QuestFactName));
        QuestFactNameHandle->SetValue(FName(**NewSelection));

        if (ComboButton.IsValid())
        {
            ComboButton->SetIsOpen(false);   
        }
    }
}

TSharedRef<ITableRow> FZWQuestFactNameCustomization::OnGenerateRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(STextBlock).Text(FText::FromString(*InItem))
        ];
}

FText FZWQuestFactNameCustomization::GetCurrentSelectionText() const
{
    TSharedPtr<IPropertyHandle> QuestFactNameHandle = PropertyHandlePtr->GetChildHandle(GET_MEMBER_NAME_CHECKED(FZWQuestFactSearchableName, QuestFactName));
    FName CurrentName;
    QuestFactNameHandle->GetValue(CurrentName);
    return FText::FromName(CurrentName);
}

void FZWQuestFactNameCustomization::OnMenuOpenChanged(bool bIsOpen)
{
    if (bIsOpen)
    {
        // Reset the text in the search bar
        if (SearchBar.IsValid())
        {
            SearchBar->SetText(FText::GetEmpty());
        }

        // Reset the options list to the full, unfiltered version
        FilteredOptions = OptionsSource;

        // Refresh the list view
        if (ListView.IsValid())
        {
            ListView->RequestListRefresh();
        }
    }
}
